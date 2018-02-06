MAYA = False
try:
    import maya.cmds as mc
    MAYA = True
except:
    pass


def isMaya():
    return MAYA


def warning(text):
    """Print warning in host application
    @param text:
    @return: None
    """
    if isMaya():
        mc.warning(text)
    else:
        raise (NotImplementedError)

def error(text):
    """Print error in host application
    @param text:
    @return: None
    """
    if isMaya():
        mc.error(text)
    else:
        raise (NotImplementedError)

