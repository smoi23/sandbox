import MySQLdb
import datetime
from quadro.web import settings

import logging

logger = logging.getLogger()


import urllib

class MysqlHandler():

	def __init__(self, **kwargs):
		self.host = kwargs.get('host')
		self.user = kwargs.get('user')
		self.passwd = kwargs.get('passwd')
		self.database = kwargs.get('database')
		self.connector = None

	def addUser(self, **kwargs):
		''' Add new user to table
		'''
		c = self._getConnector()
		if not c:
			return False

		cursor = c.cursor()

		# Add user entry
		dbKeys = settings.getKeys(settings.TABLE_USERDATA, 'name')
		addCmd = (	"INSERT INTO " + settings.TABLE_USERDATA + " "
					"(" + ",".join(dbKeys) + ")"
					"VALUES (" + str("%s, "*len(dbKeys))[0:-2] + ")")

		dataCmd = map(lambda x: kwargs.get(x), dbKeys)
		dataCmd = tuple(dataCmd)

		cursor.execute(addCmd, dataCmd)

		# Add data entry
		dbKeys = settings.getKeys(settings.TABLE_APPDATA, "name")
		addCmd = (	"INSERT INTO " + settings.TABLE_APPDATA + " "
					"(" + ",".join(dbKeys) + ")"
					"VALUES (" + str("%s, "*len(dbKeys))[0:-2] + ")")

		dataCmd = map(lambda x: kwargs.get(x), dbKeys)
		dataCmd = tuple(dataCmd)

		cursor.execute(addCmd, dataCmd)

		c.commit()
		cursor.close()
		c.close()

		return True

	def updateUser(self, user, **kwargs):
		''' Update user data
		'''
		c = self._getConnector()
		if not c:
			return False

		cursor = c.cursor()

		# update data for every database key if value exists in args
		for key in settings.getKeys(settings.TABLE_USERDATA, 'name'):
			if not kwargs.get(key):
				continue

			value = ''
			if key in settings.listValues:
				# Get curent values
				queryCmd = "SELECT " + key + " FROM " + settings.TABLE_USERDATA + " WHERE " + settings.IDENTIFIER + "='" + user + "'"
				cursor.execute(queryCmd)

				item = cursor.fetchone()
				if item:
					value = item[0] or ''

				if kwargs.get(key) in value.split(','):
					continue
	
			valueFromPost = kwargs.get(key)
			
			if not value:
				valueToInsert = "{0}".format(valueFromPost)
			else:
				valueToInsert = value + ",{0}".format(valueFromPost)


			logger.debug("Set value: {0}".format(valueToInsert))	
			
			try:
                        	setCmd = "UPDATE {0} SET {1}='{2}' WHERE {3}='{4}'".format(settings.TABLE_USERDATA, key, valueToInsert, settings.IDENTIFIER, user)
                        	cursor.execute(setCmd)
			except:
				logger.debug("except update")
	

		c.commit()
		cursor.close()
		c.close()

		return True

	def removeUser(self, user):
		''' Remove user from table
		'''
		c = self._getConnector()
		if not c:
			return False

		cursor = c.cursor()

		# Remove user entry
		removeCmd = (	"DELETE FROM " + settings.TABLE_USERDATA + " "
						"WHERE " + settings.IDENTIFIER + " = %s")

		cursor.execute(removeCmd, (user,))

		# Remove data entry
		removeCmd = (	"DELETE FROM " + settings.TABLE_APPDATA + " "
						"WHERE " + settings.IDENTIFIER + " = %s")

		cursor.execute(removeCmd, (user,))

		c.commit()
		cursor.close()
		c.close()

		return True


	def hasUser(self, user):
		''' Check if user exists
		'''
		c = self._getConnector()
		if not c:
			return False

		cursor = c.cursor()

		queryCmd = "SELECT " + settings.IDENTIFIER + " FROM " + settings.TABLE_USERDATA + " "
		cursor.execute(queryCmd)

		item = cursor.fetchone()
		while item:
			if user in item:
				cursor.close()
				c.close()
				return True
			item = cursor.fetchone()

		cursor.close()
		c.close()
		return False


	def getAppData(self, user, key):
		''' Return databse entry from table appData with key
		'''
		result = None
		c = self._getConnector()
		if not c:
			return None

		cursor = c.cursor()

		queryCmd = "SELECT " + key + " FROM " + settings.TABLE_APPDATA + " WHERE " + settings.IDENTIFIER + "='" + user + "'"
		cursor.execute(queryCmd)
		item = cursor.fetchone()
		if item:
			result = item[0]

		cursor.close()
		c.close()

		return result


	def getCredits(self, user):
		''' Returns credit count
		'''
		return self.getAppData(user, 'credits') or 0


	def getUserInfo(self, user):
		''' Return info as dictionary
		'''
		result = {}
		c = self._getConnector()
		if not c:
			return None

		cursor = c.cursor()

		#user data
		queryCmd = "SELECT * FROM " + settings.TABLE_USERDATA + " WHERE " + settings.IDENTIFIER + "='" + user + "'"
		cursor.execute(queryCmd)
		item = cursor.fetchone()
		if item:
			i = 0
			for key in settings.getKeys(settings.TABLE_USERDATA, 'name'):
				if i == len(item):
					break
				# HACK: format date to string
				value = item[i]
				if isinstance(value, datetime.date):
					value = str(value)
				result.setdefault(key, value)
				i += 1

		# app data
		queryCmd = "SELECT * FROM " + settings.TABLE_APPDATA + " WHERE " + settings.IDENTIFIER + "='" + user + "'"
		cursor.execute(queryCmd)
		item = cursor.fetchone()
		if item:
			i = 0
			for key in settings.getKeys(settings.TABLE_APPDATA, 'name'):
				if i == len(item):
					break
				result.setdefault(key, item[i])
				i += 1

		cursor.close()
		c.close()

		return result or None


	def addInvitations(self, invites, user):
		''' Add invited friends and increase credits for given user
		'''
		c = self._getConnector()
		if not c:
			return 0

		cursor = c.cursor()

		# create list
		if isinstance(invites, basestring):
			invitesList = filter(lambda x: x.strip(), invites.split(','))
		else:
			invitesList = invites

		# Get curent values
		# TODO: hard coded value here
		queryCmd = "SELECT invites FROM " + settings.TABLE_APPDATA + " WHERE " + settings.IDENTIFIER + "='" + user + "'"
		cursor.execute(queryCmd)

		value = ''
		item = cursor.fetchone()
		if item:
			value = item[0] or ''

		# subtract existing entries if same id is invited
		existsList = []
		if value:
			existsList = filter(lambda x: x.strip(), value.split(','))
			invitesList = list(set(invitesList) - set(existsList))

		if invitesList:
			value = ','.join(existsList + invitesList)
			setCmd = "UPDATE " + settings.TABLE_APPDATA + " SET invites='" + value + "' WHERE " + settings.IDENTIFIER + "='" + user + "'"
			cursor.execute(setCmd)

			# increase credits
			queryCmd = "SELECT credits FROM " + settings.TABLE_APPDATA + " WHERE " + settings.IDENTIFIER + "='" + user + "'"
			cursor.execute(queryCmd)
			item = cursor.fetchone()
			credit = 0
			if item:
				credit = item[0] or 0

			credit += len(invitesList)

			setCmd = "UPDATE " + settings.TABLE_APPDATA + " SET credits=" + str(credit) + " WHERE " + settings.IDENTIFIER + "='" + user + "'"
			cursor.execute(setCmd)

			c.commit()

		cursor.close()
		c.close()

		return len(invitesList)


	def addGuess(self, guess, user):
		''' Add a guess and decrease credits for given user
		'''
		c = self._getConnector()
		if not c:
			return False

		cursor = c.cursor()

		# Get curent values
		# TODO: hard coded value here
		queryCmd = "SELECT guesses FROM " + settings.TABLE_APPDATA + " WHERE " + settings.IDENTIFIER + "='" + user + "'"
		cursor.execute(queryCmd)

		value = ''
		item = cursor.fetchone()
		if item:
			value = '{0},'.format(item[0])
			if value == 'None,':
				value = ''
		value += str(guess)

		setCmd = "UPDATE " + settings.TABLE_APPDATA + " SET guesses='" + value + "' WHERE " + settings.IDENTIFIER + "='" + user + "'"
		cursor.execute(setCmd)

		# decrease credits
		queryCmd = "SELECT credits FROM " + settings.TABLE_APPDATA + " WHERE " + settings.IDENTIFIER + "='" + user + "'"
		cursor.execute(queryCmd)
		item = cursor.fetchone()
		credit = 0
		if item:
			credit = item[0] or 0

		credit = max(0, credit-1)

		setCmd = "UPDATE " + settings.TABLE_APPDATA + " SET credits=" + str(credit) + " WHERE " + settings.IDENTIFIER + "='" + user + "'"
		cursor.execute(setCmd)

		c.commit()
		cursor.close()
		c.close()

		return True


	def getTable(self, asList=True):
		''' Temp print
		'''
		c = self._getConnector()
		if not c:
			return ""

		cursor = c.cursor()

		# Print user entry
		queryCmd = "SELECT * FROM " + settings.TABLE_USERDATA + " "
		cursor.execute(queryCmd)

		tableList = []

		item = cursor.fetchone()
		while item:
			tableList.append('-> {0}\n'.format(item))
			item = cursor.fetchone()

		# spacer
		tableList.append('\n')

		# Print data entry
		queryCmd = "SELECT * FROM " + settings.TABLE_APPDATA + " "
		cursor.execute(queryCmd)

		item = cursor.fetchone()
		while item:
			tableList.append('-> {0}\n'.format(item))
			item = cursor.fetchone()

		cursor.close()
		c.close()

		if asList:
			return tableList

		return "".join(tableList)


	def _getConnector(self):
		''' Get and check connection object
		'''
		try:
			return MySQLdb.connect(user=self.user, passwd=self.passwd, host=self.host, db=self.database)
		except Exception as e:  # TODO: distinguish error types
			logger.error("No Connection to DB: {0}".format(e))
			return None

		# unused
		if not self.connector:
			self.connector = MySQLdb.connect(user=self.user, passwd=self.passwd, host=self.host, db=self.database)
		#elif not self.connector.is_connected():
		#	self.connector.connect()
		return self.connector
