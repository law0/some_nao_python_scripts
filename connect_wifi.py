#!/usr/bin/python
""" mandatory?

"""

import sys
import time

from naoqi import ALProxy
from naoqi import ALBroker
from naoqi import ALModule


class WifiManager(ALModule):
	""" Simple module, detect WordRecognized, then execute the order!!!
	
	"""
	
	def __init__(self, name):
		super(WifiManager, self).__init__(name) 
		self._name=name
		self._connManager = ALProxy("ALConnectionManager")
		self._wifiList={}
		self._lastRequestedWifi=""

		global memory
		memory = ALProxy("ALMemory")
		memory.subscribeToEvent("NetworkServiceInputRequired", self._name, "onPasswordRequired")

	def refresh(self):
		services = self._connManager.services()
		for serv in services:
			wifi = dict(serv)
			self._wifiList[wifi["Name"]]=wifi

	def list(self):
		self.refresh()
		for name in self._wifiList:
			wifi=self._wifiList[name]
			print(wifi["Name"] + " " + wifi["ServiceId"] + " " + "".join(wifi["Security"]))


	def connect(self, name, passphrase=None):
		self.refresh()
		self._lastRequestedWifi=name
		if passphrase:
			self._connManager.setServiceInput([["ServiceId", self._wifiList[self._lastRequestedWifi]["ServiceId"]],["Passphrase", passphrase]])
		self._connManager.connect(self._wifiList[name]["ServiceId"])


	def disconnect(self):
		self._connManager.disconnect(self._wifiList[self._lastRequestedWifi]["ServiceId"])


	def onPasswordRequired(self, key, value, message):
		""" ask for passphrase when NetworkServiceInputRequired event
	
		"""

		print(value)		
		passwd=raw_input("password?\n")
		print([["ServiceId", self._wifiList[self._lastRequestedWifi]["ServiceId"]],["Passphrase", passwd]])
		self._connManager.setServiceInput([["ServiceId", self._wifiList[self._lastRequestedWifi]["ServiceId"]],["Passphrase", passwd]])
		

	def unsubscribe(self):
		memory.unsubscribeToEvent("NetworkServiceInputRequired", self._name)

	def exit(self):
		self.unsubscribe()
		super(ALModule, self).exit()
		

broker=None
inObey=None
nao_ip="172.16.8.177"
nao_port=9559

def start(name, ip="172.16.8.177", port=9559):
	global wifiBroker
	nao_ip=ip
	nao_port=port
	wifiBroker = ALBroker("broker", "0.0.0.0", 0, nao_ip, nao_port)
	print("DON'T FORGET TO SHUTDOWN")
	global inCoWifi
	inCoWifi=WifiManager(name)
	return inCoWifi

def shutdown():
	global inCoWifi
#	inObey.unsubscribe()
	inCoWifi.exit()
	global wifiBroker
	wifiBroker.shutdown()
