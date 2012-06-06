from __future__ import division

import json
import os
import sys
import math
import time
import traceback
import colorsys

import pygtk
pygtk.require('2.0')
import glib
import gtk
import pango

import dds
from subjugator import topics


class Window(object):
    def __init__(self):
        self.debug_topic = topics.get('VisionDebug')
        self.config_topic = topics.get('VisionConfig')

    def start(self):
        self.wTree = gtk.Builder()
        self.wTree.add_from_file(os.path.join(os.path.dirname(os.path.abspath(sys.argv[0])), 'interface.glade'))
        self.wTree.connect_signals(self)
        
        window = self.wTree.get_object('window')
        window.show()
        
        self.loop()
    
    def loop(self):
        try:
            msg = self.config_topic.take()
        except dds.Error, e:
            if e.message != 'no data':
                raise
        else:
            self.wTree.get_object('config_text').get_buffer().set_text(msg['config'])
        
        try:
            msg = self.debug_topic.take()
        except dds.Error, e:
            if e.message != 'no data':
                raise
        else:
            x = gtk.gdk.PixbufLoader()
            x.write(msg['images'][0]['data'])
            x.close()
            self.wTree.get_object('image_view').set_from_pixbuf(x.get_pixbuf())
        
        self.loop_timer = glib.timeout_add(int(1/20*1000), lambda: self.loop() and False) # False prevents it from being called again

    def apply_config(self, widget):
        b = self.wTree.get_object('config_text').get_buffer()
        text = b.get_text(b.get_start_iter(), b.get_end_iter())
        try:
            json.loads(text)
        except Exception, e:
            traceback.print_exc()
            dialog = gtk.MessageDialog(
                parent=None,
                flags=gtk.DIALOG_DESTROY_WITH_PARENT,
                type=gtk.MESSAGE_INFO,
                buttons=gtk.BUTTONS_OK,
                message_format=e.message,
            )
            dialog.set_title('Invalid JSON')
            dialog.connect('response', lambda dialog, response: dialog.destroy())
            dialog.show()
        else:
            self.config_topic.send(dict(config=text))

    def window_closed(self, window):
        glib.source_remove(self.loop_timer)
        gtk.main_quit()


if __name__ == '__main__':
    Window().start()
    gtk.main()