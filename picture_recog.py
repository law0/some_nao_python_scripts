# -*- coding: utf-8 -*-

from naoqi import ALBroker
from naoqi import ALProxy
from naoqi import ALModule
import time

class picture_recog(ALModule):
	
	def __init__(self, name, *args):
		super(picture_recog,self).__init__(name, *args)
		self._name=name
		self._atts=ALProxy("ALTextToSpeech")
		global memory
		memory = ALProxy("ALMemory")
		memory.subscribeToEvent("PictureDetected", self._name, "onPictureRecognized")
		

	def onPictureRecognized(self, key, value, message):
		memory.unsubscribeToEvent("PictureDetected", self._name)
		try:
			if(value[1][0][0][1]=='T3'):
				self._atts.say("Je reconnais ce signe bizarre")
			if(value[1][0][0][1]=='SPOCK'):
				self._atts.say("Longue vie et prospérité!")
		except Exception as e:
			print(e)
		print(key, " ", value, " ", message)
		time.sleep(1)
		memory.subscribeToEvent("PictureDetected", self._name, "onPictureRecognized")

	def unsubscribe(self):
		memory.unsubscribeToEvent("PictureDetected", self._name)

	def exit(self):
		self.unsubscribe()
		super(ALModule, self).exit()
		

inBroker=None
inPicRecog=None
nao_ip="172.16.8.177"
nao_port=9559

def start(name, ip="172.16.8.177", port=9559):
	global inBroker
	nao_ip=ip
	nao_port=port
	inBroker = ALBroker("broker", "0.0.0.0", 0, nao_ip, nao_port)
	print("DON'T FORGET TO SHUTDOWN")
	global inPicRecog
	inPicRecog=picture_recog(name)
	return inPicRecog


def shutdown():
	global inPicRecog
#	inObey.unsubscribe()
	inPicRecog.exit()
	global inBroker
	inBroker.shutdown()

		
