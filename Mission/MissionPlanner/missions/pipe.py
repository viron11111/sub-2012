from subjugator import sched
from subjugator import nav
from subjugator import vision
from missionplanner import mission

import math
import dds

servo = vision.BottomVisualServo(kx=.3, ky=.3, xytol=.02, Ytol=.03, debug=True)

pipe_sels = dict(any=vision.Selector(vision.DOWN_CAMERA, 'pipe'),
                 left=vision.Selector(vision.DOWN_CAMERA, 'pipe', vision.FilterSort('angle', descending=False)),
                 right=vision.Selector(vision.DOWN_CAMERA, 'pipe', vision.FilterSort('angle', descending=True)))

def run(name):
    nav.setup()
    nav.depth(.6)

    while True:
        with sched.Timeout(20) as timeout:
            print 'Looking for ' + name + ' pipe'
            nav.vel(.2)
            vision.wait_visible(pipe_sels[name])
            print 'See pipe!'
        if timeout.activated:
            print 'Timed out on pipe'
            return False

        with mission.State('servo'):
            if servo(pipe_sels[name]):
                break

    print 'Saved last pipe position'
    mission.missiondata['last-pipe'] = nav.get_trajectory()
    return True

mission.missionregistry.register('Pipe', lambda: run('any'), 60)
mission.missionregistry.register('Pipe-left', lambda: run('left'), 60)
mission.missionregistry.register('Pipe-right', lambda: run('right'), 60)
