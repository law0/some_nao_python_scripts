from naoqi import ALProxy as ap
ttm = ap("ALMotion", "172.16.8.177",9559)
ttm.setAngles("LShoulderPitch", -.2, 0.5)
ttm.setAngles("RShoulderPitch", -.2, 0.5)
