import numbers
MAYA = False
try:
	import maya.cmds as mc
	MAYA = True
except:
	pass
	
def isMaya():
	return MAYA




class mVector(list):

	@property
	def x(self):
		return self._x

	@property
	def y(self):
		return self._y

	@property
	def z(self):
		return self._z

	@property
	def w(self):
		return self._w

	@x.setter
	def x(self, v):
		self[0] = float(v)

	@y.setter
	def y(self, v):
		self[1] = float(v)

	@z.setter
	def z(self, v):
		self[2] = float(v)

	@w.setter
	def w(self, v):
		self[3] = float(v)

	def __new__(cls, x=0.0, y=0.0, z=0.0, w=0.0):
		return list.__new__(cls, [float(x), float(y), float(z), float(w)])

	def __init__(self, x=0.0, y=0.0, z=0.0, w=0.0):
		super(mVector, self).__init__([float(x), float(y), float(z), float(w)])
		self._x, self._y, self._z, self._w = self[0], self[1], self[2], self[3]

	def __repr__(self):
		return "{_x} {_y} {_z} {_w}".format(**self.__dict__)

	def __setitem__(self, i, v):
		super(mVector, self).__setitem__(i, float(v))
		self._x, self._y, self._z, self._w = self[0], self[1], self[2], self[3]

	def __add__(self, v):
		return mVector(self[0]+v[0], self[1]+v[1], self[2]+v[2], self[3]+v[3])

	def __div__(self, n):
		if isinstance(n, numbers.Number):
			return mVector(self[0]/float(n), self[1]/float(n), self[2]/float(n), self[3]/float(n))
		else:
			raise(TypeError("Not a number: {0}".format(type(n))))

	def __mul__(self, n):
		if isinstance(n, numbers.Number):
			return mVector(self[0]*float(n), self[1]*float(n), self[2]*float(n), self[3]*float(n))
		else:
			raise(TypeError("Not a number: {0}".format(type(n))))

	def append(self, *args):
		raise(NotImplementedError)

	def extend(self, *args):
		raise(NotImplementedError)

	def insert(self, *args):
		raise(NotImplementedError)

	def pop(self, *args):
		return self.w

	def remove(self, *args):
		raise(NotImplementedError)


class DimComponents(object):

	def __init__(self):
		"""
		"""
		self.pAxis = 'x'
		self.pConstrainX = True
		self.pConstrainY = True
		self.pConstrainZ = True

#	@ismaya
	def execute(self, targetDimension, selection):
		"""
		"""
		
		bboxMin, bboxMax = self._getMinMax(selection)
		dimensions = self._getDimensions(bboxMin, bboxMax)
		currentDim = getattr(dimensions, self.pAxis)

		if currentDim < 0.001:
			mc.warning("Dimension on axis {0} is zero. Aborted.".format(self.pAxis))
			return

		# calc scale vector
		scaleFactor = targetDimension / currentDim
		scaleVector = mVector(1.0, 1.0, 1.0)
		if self.pConstrainX or self.pAxis == 'x':
			scaleVector.x = scaleFactor
		if self.pConstrainY or self.pAxis == 'y':
			scaleVector.y = scaleFactor
		if self.pConstrainZ or self.pAxis == 'z':
			scaleVector.z = scaleFactor

		# scale it
		center = (bboxMin + bboxMax) / 2.0
		center = mVector(bboxMin.x+bboxMax.x, bboxMin.y+bboxMax.y, bboxMin.z+bboxMax.z) / 2.0
		for item in selection or mc.ls(sl=True, fl=True):
			translation = mc.xform(item, q=True, translation=True, ws=True)
			mc.xform(item, ws=True, translation=((translation[0]-center.x) * scaleVector.x + center.x,
												(translation[1]-center.y) * scaleVector.y + center.y,
												(translation[2]-center.z) * scaleVector.z + center.z,))

	def _getMinMax(self, selection=None):
		"""
		"""
		items = selection or mc.ls(sl=True, fl=False)
		if not items:
			return mVector(), mVector()

		_bbox = mc.xform(items[0], q=True, boundingBox=True, ws=True)
		dimMin, dimMax = mVector(*_bbox[0:3]), mVector(*_bbox[3:6])

		for item in items[1:]:
			_bbox = mc.xform(item, q=True, boundingBox=True, ws=True)
			_dimMin, _dimMax = mVector(*_bbox[0:3]), mVector(*_bbox[3:6])
			dimMin.x = min(dimMin.x, _dimMin.x)
			dimMin.y = min(dimMin.y, _dimMin.y)
			dimMin.z = min(dimMin.z, _dimMin.z)
			dimMax.x = max(dimMax.x, _dimMax.x)
			dimMax.y = max(dimMax.y, _dimMax.y)
			dimMax.z = max(dimMax.z, _dimMax.z)

		return dimMin, dimMax

	def _getDimensions(self, i_min, i_max):
		return mVector(abs(i_min.x-i_max.x), abs(i_min.y-i_max.y), abs(i_min.z-i_max.z))

'''
Usage:
from mayaUtils import dimComponents
D = dimComponents.DimComponents()
D.execute(1.0)
'''
