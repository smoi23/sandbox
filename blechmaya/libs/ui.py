import PySide2.QtGui as QtGui
import PySide2.QtCore as QtCore
import PySide2.QtWidgets as QtWidgets

from blechmaya.libs import db


class TimeSlider(QtWidgets.QSlider):
    """Custom slider with defined key positions. Hold down ctrl button to jump to next lower key position.

        @param sizeChanged: qt signal emitted when resize event occur
        @param tickList:    list of integers defining slider key positions
    """

    sizeChanged = QtCore.Signal()
    tickList = []

    def __init__(self, *args, **kwargs):
        super(TimeSlider, self).__init__(*args, **kwargs)
        self.setOrientation(QtCore.Qt.Orientation.Horizontal)
        self.setFocusPolicy(QtCore.Qt.StrongFocus)
        self.isCtrlDown = False
        self.setSingleStep(1)
        self.connectInternal()

    def resizeEvent(self, event):
        """Override. Receive resize event and simply emit this as signal.
        @param event: passed qt event
        @return: None
        """
        self.sizeChanged.emit()
        super(TimeSlider, self).resizeEvent(event)

    def connectInternal(self):
        """Connect qt action triggered signal with callback
        @return: None
        """
        self.actionTriggered.connect(self.handleAction)

    @QtCore.Slot(int)
    def handleAction(self, action):
        """Check of the ctrl button is hold down and jump to slider position defined in tick list. Otherwise set the value without changes.
        @param action: qt action enum
        @return: None
        """
        if not action == QtWidgets.QAbstractSlider.SliderNoAction:
            _value = self.sliderPosition()
            if self.isCtrlDown:
                _ticks = filter(lambda x: x <= _value, self.tickList)
                if len(_ticks) > 0:
                    _value = sorted(_ticks).pop()
            self.setValue(_value)

    def keyPressEvent(self, event):
        """Override. Store whether ctrl button is pressed on key press.
        @param event: qt event
        @return: None
        """
        if event.key() == QtCore.Qt.Key_Control:
            self.isCtrlDown = True
        QtWidgets.QWidget.keyPressEvent(self, event)

    def keyReleaseEvent(self, event):
        """Override. Store whether ctrl button is pressed on key release.
        @param event: qt event
        @return: None
        """
        if event.key() == QtCore.Qt.Key_Control:
            self.isCtrlDown = False
        QtWidgets.QWidget.keyPressEvent(self, event)

    def focusInEvent(self, event):
        """Override. On focus check if the ctrl button already is hold down.
        @param event: qt event
        @return: None
        """
        if QtWidgets.QApplication.keyboardModifiers() == QtCore.Qt.ControlModifier:
            self.isCtrlDown = True
        else:
            self.isCtrlDown = False


class ProdSelectWidget(QtWidgets.QWidget):

    hbox = None
    showList = None
    sceneList = None
    shotList = None


    def __init__(self, *args, **kwargs):
        super(ProdSelectWidget, self).__init__(*args, **kwargs)
        self.createUI()
        self.setShows(db.query_shows())

    def createUI(self):
        """Create necessary layouts and widgets. All ui elements will be aligned together.
        @return: None
        """
        self.createDropdownSelect()
        self.createConnections()

    def createDropdownSelect(self):
        """Create three dropdown selection lists for show, scene and shot
        @return: None
        """

        # horizontal layout
        self.hbox = QtWidgets.QHBoxLayout()

        # dropdown shows scenes shots
        self.hbox.addWidget(QtWidgets.QLabel("Show"))
        self.showList = QtWidgets.QComboBox(self)
        self.hbox.addWidget(self.showList)
        self.hbox.addWidget(QtWidgets.QLabel("Scene"))
        self.sceneList = QtWidgets.QComboBox(self)
        self.hbox.addWidget(self.sceneList)
        self.hbox.addWidget(QtWidgets.QLabel("Shot"))
        self.shotList = QtWidgets.QComboBox(self)
        self.hbox.addWidget(self.shotList)

        # assign layout
        self.setLayout(self.hbox)

    def createConnections(self):
        """Connect signals and slots within this UI.
        @return:
        """
        # Combo boxes
        self.showList.currentIndexChanged[str].connect(self.handleShowSelected)
        self.sceneList.currentIndexChanged[str].connect(self.handleSceneSelected)
        self.shotList.currentIndexChanged[str].connect(self.handleShotSelected)

    def setShows(self, items):
        """Set list string values
        @param items: list of strings
        @return: None
        """
        self.showList.clear()
        self.showList.addItems(items)

    def setScenes(self, items):
        """Set list string values
        @param items: list of strings
        @return: None
        """
        self.sceneList.clear()
        self.sceneList.addItems(items)

    def setShots(self, items):
        """Set list string values
        @param items: list of strings
        @return: None
        """
        self.shotList.clear()
        self.shotList.addItems(items)

    @QtCore.Slot(str)
    def handleShowSelected(self, selection):
        """Get and set scenes according to selected show
        @param selection: string with current selection
        @return: None
        """
        self.setScenes(db.query_scenes(selection))

    @QtCore.Slot(str)
    def handleSceneSelected(self, selection):
        """Get and set shows according to selected show
        @param selection: string with current selection
        @return: None
        """
        show = self.showList.currentText()
        self.setShots(db.query_shots(show, selection))

    @QtCore.Slot(str)
    def handleShotSelected(self, selection):
        """Currently does nothing
        @param selection: string with current selection
        @return: None
        """
        pass
