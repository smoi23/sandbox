""" Camera Sequencer Util
    Author: Andreas Schuster
    License:

    Description:
    Utility to place shot cameras in time line and shuffle through them. Currently only implemented for Maya.

    Usage:
    Source python directories and import module 'CameraSequencer'

    Run in script editor: cs =  CameraSequencer.CameraSequencer()
    Shot cameras within the scene are placed in the list.

    Select Show, Scene (and shot) and press Import/Import All. This will
    get shot information from db-module and create cameras in your scene.

    Use the slider to scroll the time line. Look through will change depending on camera's range and the current frame.

    Double click for look through immediately. This will not change the time.

    Click on the icon to activate/deactivate camera.

    Hold ctrl when using the slider will jump to cameras start position.

"""
import os

from blechmaya.libs import db
from blechmaya.libs import system
from blechmaya.libs import ui
from blechmaya.libs.decorators import *

import PySide2.QtGui as QtGui
import PySide2.QtCore as QtCore
import PySide2.QtWidgets as QtWidgets

from shiboken2 import wrapInstance

if system.isMaya():
    import maya.cmds as mc
    import maya.OpenMayaUI


ICON_HEIGHT = 16
ICON_VISIBLE = os.path.join(os.path.dirname(__file__), "icon_visible.png")
ICON_INVISIBLE  = os.path.join(os.path.dirname(__file__), "icon_invisible.png")
ICON_TEXTURE_PREVIEW_DIM = 16
COLORFIELD_PADDING = 2
COLORFIELD_WIDTH = 40
FOCALFIELD_WIDTH = 60
RANGEFIELD_WIDTH = 80

def mayaMainWindow():
    """Returns the Maya window as a QMainWindow instance.
    """
    ptr = maya.OpenMayaUI.MQtUtil.mainWindow()
    if ptr is not None:
        return wrapInstance(long(ptr), QtWidgets.QWidget)



class CameraItem(QtGui.QStandardItem):
    """Item holding the camera data and used to hook into data model

        @param id: db id of the camera item
        @param transform: dag (uuid) to the transform
        @param shape: dag (uuid) to the shape
        @param offset: difference to timeline slider position
        @param focalLength: cameras focal length
    """

    id = ""
    transform = None
    shape = None
    isActive = True
    offset = 0
    focalLength = 0
    __range = (0, 0)

    def __init__(self, id, *args, **kwargs):
        super(CameraItem, self).__init__(*args, **kwargs)
        self.id = id
        self.setEditable(False)

    @property
    def range(self):
        """Tuple of two values with cut in/out (include borders) in which the camera item supposed to be active
        """
        return self.__range

    @range.setter
    def range(self, i_range):
        assert isinstance(i_range, (tuple, list)), "Argument is not Tuple or List"
        assert len(i_range) > 1, "List needs two values"
        self.__range = i_range

    def isInRange(self, frame):
        """Indicate whether the camera is active in give frame or not
        @param frame: frame number to check
        @return: boolean result
        """
        if frame in range(self.__range[0]+self.offset, self.__range[1]+self.offset+1):
            return True
        return False


class CameraSequencerUIView(QtWidgets.QListView):
    """List view to hold camera item objects

        @param mousePressPosX: storage for the mouse x-position when item is pressed to distinguish actions
    """

    visibilitySelectArea = (0,0)
    visibilityPressed = False

    def __init__(self, *args, **kwargs):
        super(CameraSequencerUIView, self).__init__(*args, **kwargs)
        self.visibilitySelectArea = (self.width()-COLORFIELD_WIDTH - ICON_HEIGHT, self.width() - ICON_HEIGHT)


    def resizeEvent(self, event):
        """Override. Receive resize event and update visibilitySelectArea.
        @param event: passed qt event
        @return: None
        """
        self.visibilitySelectArea = (self.width()-COLORFIELD_WIDTH - ICON_HEIGHT, self.width() - ICON_HEIGHT)
        super(CameraSequencerUIView, self).resizeEvent(event)


    def mousePressEvent(self, event):
        """Override. Store mouse x-position.
        @param event: qt mouse event
        @return: None
        """
        mousePressPosX = event.x()
        self.visibilityPressed = (mousePressPosX > self.visibilitySelectArea[0] and mousePressPosX < self.visibilitySelectArea[1])
        super(CameraSequencerUIView, self).mousePressEvent(event)


class CameraSequencerUIModel(QtGui.QStandardItemModel):
    """Data model for QtListView
    """

    def __init__(self, *args, **kwargs):
        super(CameraSequencerUIModel, self).__init__(*args, **kwargs)


class CameraItemDelegate(QtWidgets.QItemDelegate):
    """Delegate to be used in standard item model with custom painting.
    """
    def __init__(self, *args, **kwargs):
        super(CameraItemDelegate, self).__init__(*args, **kwargs)

    def paint(self, painter, option, index):
        """Override. Get the item from the model and draw custom paintings
        @param painter: qt painter
        @param option: qt option (draw area)
        @param index: qt index pointing into the model
        @return: None
        """
        # get item
        item = self.parent().model().itemFromIndex(index)
        #item = index.internalPointer()
        #name, value = self.parent().model().data(index, QtCore.Qt.DisplayRole)

        # draw row backgroundcolor
        painter.save()
        if option.state & QtWidgets.QStyle.State_Selected:
            painter.setBrush(self.parent().palette().highlight())
            painter.setPen(QtGui.QPen(QtCore.Qt.NoPen))
            painter.drawRect(option.rect)
        else:
            if index.row() % 2 == 1:
                painter.setBrush(self.parent().palette().midlight())
            painter.setPen(QtGui.QPen(QtCore.Qt.NoPen))
            painter.drawRect(option.rect)

        painter.restore()

        painter.save()

        # change font size
        #_font = painter.font()
        #_font.setPixelSize(10)
        #painter.setFont(_font)

        if not item.isActive:
            painter.setPen(QtGui.QPen(QtGui.QColor(255, 100, 100)))

        # name
        currentColumnWidth = ICON_HEIGHT
        painter.drawText(QtCore.QRect(option.rect.x() + currentColumnWidth, option.rect.y(), option.rect.width() - RANGEFIELD_WIDTH - FOCALFIELD_WIDTH - COLORFIELD_WIDTH - ICON_HEIGHT,
                                      option.rect.height()), QtCore.Qt.AlignLeft | QtCore.Qt.AlignVCenter, item.text())

        # range
        currentColumnWidth += option.rect.width() - RANGEFIELD_WIDTH - FOCALFIELD_WIDTH - COLORFIELD_WIDTH - ICON_HEIGHT
        painter.drawText(QtCore.QRect(option.rect.x() + currentColumnWidth, option.rect.y(), RANGEFIELD_WIDTH,
                                      option.rect.height()), QtCore.Qt.AlignLeft | QtCore.Qt.AlignVCenter, "{0} {1}".format(item.range[0], item.range[1]))

        # focal length
        currentColumnWidth += RANGEFIELD_WIDTH
        painter.drawText(QtCore.QRect(option.rect.x() + currentColumnWidth, option.rect.y(), FOCALFIELD_WIDTH,
                                      option.rect.height()), QtCore.Qt.AlignLeft | QtCore.Qt.AlignVCenter, str(item.focalLength))

        # visibility icon
        currentColumnWidth += FOCALFIELD_WIDTH
        if item.isActive:
            color = (0.1,0.6,0.1)
            pixmap = self.getPixmap(ICON_VISIBLE, ICON_TEXTURE_PREVIEW_DIM, ICON_TEXTURE_PREVIEW_DIM)
        else:
            color = (0.6,0.1,0.1)
            pixmap = self.getPixmap(ICON_INVISIBLE, ICON_TEXTURE_PREVIEW_DIM, ICON_TEXTURE_PREVIEW_DIM)

        if not pixmap.isNull():
            painter.drawImage(QtCore.QRect(option.rect.x() + currentColumnWidth, option.rect.y(), ICON_TEXTURE_PREVIEW_DIM, option.rect.height()), pixmap.toImage())
        else:
            painter.fillRect(QtCore.QRect(option.rect.x() + currentColumnWidth,
                             option.rect.y() + COLORFIELD_PADDING,
                             COLORFIELD_WIDTH,
                             option.rect.height() - COLORFIELD_PADDING - COLORFIELD_PADDING),
                             QtGui.QColor.fromRgbF(color[0], color[1], color[2]))

        painter.restore()


    def getPixmap(self, src, width=64, height=64):
        """Create pixmap from source and with given dimension
        @param src: image location
        @param width: width of the pixmap
        @param height: height of the pixmap
        @return: QtGui.QPixmap
        """
        pm = QtGui.QPixmap()
        if not QtGui.QPixmapCache.find(src, pm):
            res = pm.load(src, flags=QtCore.Qt.ColorOnly)
            if not res:
                return QtGui.QPixmap()
            pm = pm.scaled(QtCore.QSize(width, height))
            QtGui.QPixmapCache.insert(src, pm)
        return pm

    '''
    def createEditor(self, parent, option, index):
        # print "createEditor"
        item = index.internalPointer()
        editor = QtGui.QComboBox(parent)
        editor.activated.connect(editor.close)
        return editor


    def setEditorData(self, editor, index):
        # print "setEditorData"
        item = index.internalPointer()
        editor.setCurrentIndex(idx)


    def setModelData(self, editor, model, index):
        # print "setModelData"
        item.nodeValue = editor.currentIndex()


    def updateEditorGeometry(self, editor, option, index):
        item = index.internalPointer()
        rectWidth = option.rect.x() + ICON_HEIGHT + TEXT_HEIGHT * self.longestAttributeCharCount + COLORFIELD_WIDTH
        rect = QtCore.QRect(rectWidth,
                            option.rect.y(),
                            self.parent().width() - rectWidth - ICON_HEIGHT,
                            option.rect.height())
        editor.setGeometry(rect)
    '''


class CameraSequencerUI(QtWidgets.QMainWindow):
    """Main window for camera sequencer
    """
    centralwidget = None
    cameralist = None
    prodselect = None
    vbox = None
    buttonCamera = None
    buttonCameraAll = None
    timeline = None

    cameraRequestSignal = QtCore.Signal( [str,str,str], [str,str])
    cameraSelectedSignal = QtCore.Signal(CameraItem)
    cameraVisibilityChanged = QtCore.Signal()

    def __init__(self, *args, **kwargs):
        super(CameraSequencerUI, self).__init__(*args, **kwargs)
        self.createUI()
        self.setWindowTitle("Camera Sequencer")
        self.createConnections()

    def createUI(self):
        """Create necessary layouts and widgets. All ui elements will be aligned together.
        @return: None
        """
        self.centralwidget = QtWidgets.QWidget()
        self.setCentralWidget(self.centralwidget)

        self.vbox = QtWidgets.QVBoxLayout()

        # production selection
        self.prodselect = ui.ProdSelectWidget(self.centralwidget)
        self.vbox.addWidget(self.prodselect)
        # add stretch
        self.prodselect.hbox.addStretch(1)
        # buttons
        self.buttonCamera = QtWidgets.QPushButton("Import Camera", self.prodselect)
        self.prodselect.hbox.addWidget(self.buttonCamera)
        self.buttonCameraAll = QtWidgets.QPushButton("Import All Cameras",  self.prodselect)
        self.prodselect.hbox.addWidget(self.buttonCameraAll)

        # list view
        self.cameralist = CameraSequencerUIView(self.centralwidget)
        self.cameralist.setStyleSheet("border: 1px solid red")
        self.cameralist.setSizePolicy(QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Expanding)

        # list model
        model = CameraSequencerUIModel(parent=self.cameralist)
        self.cameralist.setModel(model)

        # list delegate
        cameradelegate = CameraItemDelegate(self.cameralist)
        self.cameralist.setItemDelegate(cameradelegate)

        # camera list widget
        self.vbox.addWidget(self.cameralist)

        # time slider
        self.timeline = ui.TimeSlider(self.centralwidget)
        self.timeline.setStyleSheet("border: 1px solid yellow")
        self.vbox.addWidget(self.timeline)

        # assign layout to central widget
        self.centralwidget.setLayout(self.vbox)

    def createConnections(self):
        """Connect signals and slots within this UI.
        @return:
        """
        # Buttons
        self.buttonCamera.clicked.connect(self.handleCreateCameraClicked)
        self.buttonCameraAll.clicked.connect(self.handleCreateAllClicked)
        # list view
        self.cameralist.clicked.connect(self.handleItemClicked)
        self.cameralist.doubleClicked.connect(self.handleItemDoubleClicked)
        # timesline
        self.timeline.sizeChanged.connect(self.updateLabelPositions)

    @QtCore.Slot()
    def handleItemClicked(self, index):
        """Callback on item click. Update UI with camera information if wanted.
            Check at list view if visibility icon was pressed. Negate the active state from camera item.
        @param index: qt model index
        @return: None
        """
        if self.cameralist.visibilityPressed:
            item = self.cameralist.model().itemFromIndex(index)
            item.isActive = not item.isActive
            self.cameralist.repaint()
            self.cameraVisibilityChanged.emit()

    @QtCore.Slot()
    def handleItemDoubleClicked(self, index):
        """Callback on item double click. Emit new signal with clicked item as argument.
        @param index: qt model index
        @return: None
        """
        item = self.cameralist.model().itemFromIndex(index)
        self.cameraSelectedSignal.emit(item)

    @QtCore.Slot()
    def handleCreateCameraClicked(self):
        """Button callback for creating one camera. Emit new signal with show, scene, shot)
        @return: None
        """
        print self.prodselect.showList.currentText(), self.prodselect.sceneList.currentText(), self.prodselect.shotList.currentText()
        self.cameraRequestSignal[str,str,str].emit(self.prodselect.showList.currentText(), self.prodselect.sceneList.currentText(), self.prodselect.shotList.currentText())

    @QtCore.Slot()
    def handleCreateAllClicked(self):
        """Button callback for creating all camera from one scene. Emit new signal with show, scene)
        @return: None
        """
        print self.prodselect.showList.currentText(), self.prodselect.sceneList.currentText()
        self.cameraRequestSignal[str,str].emit(self.prodselect.showList.currentText(), self.prodselect.sceneList.currentText())


    def setListItems(self, items):
        """Clear current items and set new item objects to the model. Create a label for every camera item.
        @param items: list of camera items
        @return: None
        """
        # clear
        self.cameralist.model().clear()

        # append
        for item in items:
            self.cameralist.model().appendRow(item)

        # remove previous labels
        for child in self.timeline.children():
            child.deleteLater()

        # add new labels
        for i in range(len(items)):
            if not items[i].isActive:
                continue
            _label = QtWidgets.QLabel("Cam " + str(i), self.timeline)
            _label.show()
            _label.setStyleSheet("border: 0px none yellow")

        # re-postion labels
        self.updateLabelPositions()


    @QtCore.Slot()
    def updateLabelPositions(self):
        """Re-position labels on the timeline slider according to the start value ot the cameras range. Update the ticklist with key position.
            This is called on inserting of new items and on resize events.
        @return:
        """
        self.timeline.tickList = []
        for i, child in enumerate(self.timeline.children()):
            item = self.cameralist.model().item(i)
            if not item:
                continue
            posX = float(item.range[0] + item.offset) / float(self.timeline.maximum()) * float(self.timeline.width())
            child.move(posX, 0)
            # append to key position list
            self.timeline.tickList.append(item.range[0] + item.offset)

    def setTimelineSliderRange(self, v1, v2):
        """Set range on slider widget.
        @param v1: min
        @param v2: max
        @return: None
        """
        self.timeline.setRange(v1,v2)


class CameraSequencer(object):
    """Tool to bring shot cameras in one sequence
    """
    ui = None
    defaultCameras = ["persp", "top", "front", "side"]
    cameras = []
    currentCamera = None

    def __init__(self):
        mainWindow = self.__getMainWindow()

        # try find ui
        self.ui = mainWindow.findChild(QtWidgets.QWidget, "__blechmaya_camerasequencer")

        if self.ui:
            self.ui.destroy()

        # create
        self.ui = CameraSequencerUI(parent=mainWindow)
        self.ui.setObjectName("__blechmaya_camerasequencer")

        # connect callbacks with signals from UI class
        self.ui.cameraRequestSignal[str,str].connect(self.createCamerasFromDB)
        self.ui.cameraRequestSignal[str,str,str].connect(self.createCameraFromDB)
        self.ui.cameraSelectedSignal.connect(self.lookThrough)
        self.ui.timeline.valueChanged.connect(self.updateActiveCamera)
        self.ui.cameraVisibilityChanged.connect(self.populateCameras)

        # show and focus
        self.ui.show()

        self.readCamerasFromScene()

    def __getMainWindow(self):
        """Receive main window from host application
        @return: None
        """
        if system.isMaya():
            return mayaMainWindow()
        else:
            raise(NotImplementedError)

    @func_debug_decorator
    @QtCore.Slot(str, str)
    def createCamerasFromDB(self, show, scene):
        """Loop through shots and call method to create camera and populate changes to ui
        @param show: show name
        @param scene: scene name
        @return: None
        """
        for s in db.query_shots(show, scene):
            self._createCamera(show, scene, s)

        self.populateCameras()

    @QtCore.Slot(str, str, str)
    def createCameraFromDB(self, show, scene, shot):
        """Call method to create camera and populate changes to ui
        @param show: show name
        @param scene: scene name
        @param shot:  shot name
        @return: None
        """
        self._createCamera(show, scene, shot)
        self.populateCameras()

    def _createCamera(self, show, scene, shot):
        """Create a camera in the application based on shot info and put a new camera item in list. Don't create if already in ui.
        @param show: show name
        @param scene: scene name
        @param shot:  shot name
        @return: None
        """
        # get db object
        cameraDB = db.query_asset_camera(show, scene, shot)

        # check if camera already there and return
        if self.isCameraInList(cameraDB.get('id')):
            system.warning("Camera already in scene")
            return None

        # create
        if system.isMaya():
            cam = mc.camera(name=db.query_name(show, scene, shot))
        else:
            raise (NotImplementedError)

        # set attributes at shape node
        self.setCameraAttributes(cam[1], {'cmp_show': show, 'cmp_scene': scene, 'cmp_shot': shot})

        # list item
        camItem = CameraItem(cameraDB.get('id'), cam[0])
        camItem.range = db.query_camera_range(show, scene, shot)
        camItem.focalLength = db.query_camera_focallength(show, scene, shot)
        # path to nodes
        if system.isMaya():
            camItem.transform = mc.ls(cam[0], uuid=True)[0]
            camItem.shape = mc.ls(cam[1], uuid=True)[0]
        else:
            raise (NotImplementedError)

        # add to list
        self.cameras.append(camItem)


    def readCamerasFromScene(self, selected=False):
        """List all cameras in scene and check if they contains production attribute. If all attribute are set, put a new camera item in list.
        Continues without new camera item if already in ui.
        @param selected: only consider selected nodes
        @return: None
        """
        # list cameras
        if system.isMaya():
            cams = mc.ls(type="camera", sl=selected, l=True)
        else:
            raise (NotImplementedError)

        for cam in cams:
            # get attributes and continue if one was not found at the camera. assuming its not a shot camera
            context = self.getCameraAttributes(cam, {'cmp_show': None, 'cmp_scene': None, 'cmp_shot': None})
            if not context:
                continue

            # get db object
            cameraDB = db.query_asset_camera(context['cmp_show'], context['cmp_scene'], context['cmp_shot'])

            # check if camera already there
            if self.isCameraInList(cameraDB.get('id')):
                system.warning("Camera already in scene")
                continue

            # create list item
            camItem = CameraItem(cameraDB.get('id'), mc.listRelatives(cam, parent=True, f=False)[0])
            camItem.range = cameraDB.get('range')
            camItem.focalLength = cameraDB.get('focallength')
            if system.isMaya():
                camTransform = mc.listRelatives(cam, parent=True, f=True)[0]
                camItem.transform = mc.ls(camTransform, uuid=True)[0]
                camItem.shape = mc.ls(cam, uuid=True)[0]
            else:
                raise (NotImplementedError)

            # add to list
            self.cameras.append(camItem)

        self.populateCameras()

    def isCameraInList(self, id):
        """Check if camera is already in tool's list
        @param id: db id of the asset
        @return: boolean result
        """
        for cam in self.cameras:
            if cam.id == id:
                return True
        return False

    @QtCore.Slot(CameraItem)
    def lookThrough(self, cam):
        """Look through given camera.
        @param cam: path to camera
        @return: None
        """
        if system.isMaya():
            dag = mc.ls(cam.transform, l=True)
            mc.lookThru(dag)
        else:
            raise (NotImplementedError)


    @QtCore.Slot()
    def populateCameras(self):
        """Set camera items at ui's view widget and call update ranges.
        @return: None
        """
        self.updateRanges()
        self.ui.setListItems(self.cameras)

    @QtCore.Slot(int)
    def updateActiveCamera(self, frame):
        """Check if there is an current camera and if frame fits in it's range. Switch to next matching camera, if frame is outside current camera's range.
        @param frame: usually the frame value from timeline slider
        @return: None
        """
        if not self.currentCamera or not self.currentCamera.isInRange(frame):
            for c in self.cameras:
                if c.isInRange(frame):
                    self.currentCamera = c
                    self.lookThrough(self.currentCamera)
                    break

        # set app timeline
        if system.isMaya():
            mc.currentTime(frame-self.currentCamera.offset)
        else:
            raise(NotImplementedError)

    def setCameraAttributes(self, node, context):
        """Write camera information at the camera node within application
        @param node: path to the node shape or transform
        @param context: dictionary with production information and id of the camera asset
        @return: None
        """
        assert isinstance(context, dict), "Argument not of type dictionary"
        for k,v in context.items():
            assert isinstance(v, basestring), "Value not of type string"
            if system.isMaya():
                if not mc.objExists("{0}.{1}".format(node, k)):
                    mc.addAttr(node, shortName=k, longName=k, dt="string", readable=True, keyable=False,  writable=False, hidden=False)
                mc.setAttr("{0}.{1}".format(node, k), v, type="string")
            else:
                raise(NotImplementedError)

    def getCameraAttributes(self, node, context):
        """Read camera information at the camera node within application
        @param node: path to the node shape or transform
        @param context: dictionary with production information to look for
        @return: dictionary context with attributes or None if one(!) is missing
        """
        assert isinstance(context, dict), "Argument not of type dictionary"
        context_read = {}
        for k in context.keys():
            if system.isMaya():
                if not mc.objExists("{0}.{1}".format(node, k)):
                    return None
                context_read[k] = mc.getAttr("{0}.{1}".format(node, k))
            else:
                raise(NotImplementedError)
        return context_read


    @func_debug_decorator
    def updateRanges(self):
        """Walk through all active cameras and set offset properly. This will place the cameras one after the other.
            Set the timeline slider range to [0, accumulated ranges]
        @return: None
        """
        #  list of active cameras
        activeCameras = filter(lambda x: x.isActive, self.cameras)

        if len(activeCameras) == 0:
            return

        # adjust offsets
        maxValue = 0
        for cam in activeCameras:
            if not cam.isActive:
                continue
            cam.offset = maxValue-cam.range[0]
            maxValue += cam.range[1]-cam.range[0] + 1

        # set slider ranges
        self.ui.setTimelineSliderRange(0, maxValue)

