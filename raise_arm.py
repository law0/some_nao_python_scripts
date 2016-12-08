#!/usr/bin/python
""" mandatory?

"""

import sys
import time

from naoqi import ALProxy
from naoqi import ALBroker
from naoqi import ALModule

class RaiseArmModule(ALModule):
	""" Simple module, detect facedetection and raise arms
	
	"""
	
	def __init__(self, name):
		ALModule.__init__(self, name)
		self._name=name

		self.mover=ALProxy("ALMotion")
		self.mover.setStiffnesses("LShoulderPitch", 1.0)
		self.mover.setStiffnesses("RShoulderPitch", 1.0)

		self.tts=ALProxy("ALTextToSpeech")

		global memory
		memory = ALProxy("ALMemory")
		memory.subscribeToEvent("FaceDetected", self._name, "raiseThem")

	def raiseThem(self, *_args):
		""" Raise arms, when FaceDetected event
	
		"""
		
		memory.unsubscribeToEvent("FaceDetected", self._name)
		print(_args)
		self.tts.say("I see you")
		self.mover.post.setAngles("LShoulderPitch", -.2, 0.1)
		self.mover.setAngles("RShoulderPitch", -.2, 0.1)
		self.mover.waitUntilMoveIsFinished()

		#memory.subscribeToEvent("FaceDetected", self._name, "raiseThem")

broker = ALBroker("broker", "0.0.0.0", 0, "172.16.8.177", 9559)

def start():
	
	try:
		while True:
			time.sleep(1)
	
	except KeyboardInterrupt:
		print()
		print("Goodbye")
		broker.shutdown()
	

