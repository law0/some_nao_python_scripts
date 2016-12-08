#!/usr/bin/python
""" mandatory?

"""

import sys
import time

from naoqi import ALProxy
from naoqi import ALBroker
from naoqi import ALModule
import re

class Obey(ALModule):
	""" Simple module, detect WordRecognized, then execute the order!!!
	
	"""
	
	def __init__(self, name):
		#ALModule.__init__(self,name)
		super(Obey, self).__init__(name) #<----- does that works? yes, when correct hahaha
		self._name=name

		self.auto_move=ALProxy("ALAutonomousMoves")
		self.auto_move.setExpressiveListeningEnabled(False)

		self.tts=ALProxy("ALTextToSpeech")
		self.pp=ALProxy("ALRobotPosture")
		self.mover=ALProxy("ALMotion")
		self.mover.setStiffnesses("Body",1.0)
		self.sr=ALProxy("ALSpeechRecognition")
		self.sr.setLanguage("French")
		self.sr.setVocabulary(["assis","debout","accroupi","bras"], True)
		self.sr.subscribe("obey")		
		self.regex = re.compile(".*(assis|debout|accroupi|bras).*")

		global memory
		memory = ALProxy("ALMemory")
		memory.subscribeToEvent("WordRecognized", self._name, "onWordRecognized")

	def onWordRecognized(self, key, value, message):
		""" react and execute order
	
		"""
		

		self.sr.unsubscribe("obey")		
		memory.unsubscribeToEvent("WordRecognized", self._name)

		print(value)
		i=0
		while i<len(value):
			res = self.regex.match(value[i])
			if res:
				print(res.group(1))
				if value[i+1]>=0.30:
					if res.group(1)=="assis":
						self.pp.goToPosture("Sit", 0.6)
					elif res.group(1)=="debout":
						self.pp.goToPosture("StandInit", 0.6)
					elif res.group(1)=="accroupi":
						self.pp.goToPosture("Crouch", 0.6)
					elif res.group(1)=="bras":
						self.pp.goToPosture("StandZero", 0.6)
					break
			else:
				print(value[i])
			
			i=i+2
		self.mover.waitUntilMoveIsFinished()
		self.sr.subscribe("obey")		
		memory.subscribeToEvent("WordRecognized", self._name, "onWordRecognized")
		
	
	def unsubscribe(self):
		self.sr.unsubscribe("obey")		
		memory.unsubscribeToEvent("WordRecognized", self._name)

	def exit(self):
		self.unsubscribe()
		super(ALModule, self).exit()
		

broker=None
inObey=None
nao_ip="172.16.8.177"
nao_port=9559

def start(name, ip="172.16.8.177", port=9559):
	global broker
	nao_ip=ip
	nao_port=port
	broker = ALBroker("broker", "0.0.0.0", 0, nao_ip, nao_port)
	print("DON'T FORGET TO SHUTDOWN")
	global inObey
	inObey=Obey(name)
	return inObey

def shutdown():
	global inObey
#	inObey.unsubscribe()
	inObey.exit()
	global broker
	broker.shutdown()
