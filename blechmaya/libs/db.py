

# show/scene/shot
fakeDB = {  'abc':
              {'scene1':
                   {'shot1': {'info': {'name': "room_close",
                                       'range': (20010, 20090)},
                              'camera': {'range': (10,80),
                                        'focallength': 50,
                                         'id': "bb649c83dd1ea5c9d9dec9a18df0ffe9"
                                        },
                              'plates:': "606d4a1972f511d9a320251bbf9d07a1"
                              },
                    'shot2':  {'info': {'name': "room_total"},
                               'camera': {'range': (10,40),
                                        'focallength': 30,
                                        'id': "bb649c83dd1ea5c9d9dec9a18df0ffe8"
                                          },
                              'plates:': "606d4a1972f511d9a320251bbf9d07a2",
                               },
                    'shot3': {'info': {'name': "room_face"},
                              'camera':{'range': (70,180),
                                        'focallength': 70,
                                        'id': "bb649c83dd1ea5c9d9dec9a18df0ffa2"
                                        },
                              'plates:': "606d4a1972f511d9a320251bbf9d07a3"}
                    },
                'scene2':
                   {'shot1': {'info': {'name': "forest_close"},
                              'camera': {'range': (10,80),
                                        'focallength': 50,
                                        'id': "bb649c83dd1ea5c9d9dec9a18df0ffa3"
                                         },
                              'plates:': "606d4a1972f511d9a320251bbf9d07a5"
                              },
                    'shot2':  {'info': {'name': "forest_total"},
                               'camera': {'range': (81,110),
                                        'focallength': 30,
                                        'id': "bb649c83dd1ea5c9d9dec9a18df0ffa8"
                                          },
                              'plates:': "606d4a1972f511d9a320251bbf9d07a7",
                               },
                    'shot3': {'info': {'name': "forest_face"},
                              'camera':{'range': (111,200),
                                        'focallength': 70,
                                        'id': "bb649c83dd1ea5c9d9dec9a18df0afe9"
                                        },
                              'plates:': "606d4a1972f511d9a320251bbf9d07a9"}
                    },
               'scene3': {},
               'scene4': {}
               },
          'idm':
              {'scene1': {},
                'scene2': {},
               'scene3': {}
               },
          'thb':
              {'scene1': {},
               'scene2': {},
               'scene3': {}
               }
          }



def query_name(show, scene, shot):
    return fakeDB[show][scene][shot]['info']['name']

def query_asset_camera(show, scene, shot):
    return fakeDB[show][scene][shot]['camera']

def query_camera_range(show, scene, shot):
    return fakeDB[show][scene][shot]['camera']['range']

def query_camera_focallength(show, scene, shot):
    return fakeDB[show][scene][shot]['camera']['focallength']

def query_shows():
    res = fakeDB.keys()
    res.sort()
    return res

def query_scenes(show):
    if not show.strip():
        return []
    else:
        res = fakeDB[show].keys()
        res.sort()
        return res


def query_shots(show, scene):
    if not show.strip() or not scene.strip():
        return []
    else:
        res = fakeDB[show][scene].keys()
        res.sort()
        return res

def query_shotByName(show, scene, name):
    for k,v in fakeDB[show][scene].items():
        if v['info']['name'] == name:
            return k




