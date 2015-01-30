
openGL = {
	versionMajor = 4,
	versionMinor = 4
}

window = {
	name = "Battle",
	width = 1280,
	height = 960
}
maxFPS = "120"


mode = "DEBUG" -- DEBUG or NORMAL
playIntro = true


-- specifications for debug objects like the IK helper balls, normals being displayed and so on
-- these object only appear if debug mode is enabled
debug = {
	-- vertexShader = "debug",
	-- fragmentShader = "debug",

	globalCoordAxis = {
		enable = false,
		scale = 10,  
		model = "fixedAxis"
	},
	localCoordAxis = {
		enable = false,
		scale = 0.2, 
		model = "fixedAxis"
	},
	boneCoordAxis = { -- not implemented
		enable = false,
		scale = 0.3, 
		model = "fixedAxis"
	},
	ikTarget = {
		enable = true,
		scale = 1.5,
		model = "smallBall"
	},
	curvePoint = {
		enable = true,
		scale = 0.2,
		model = "smallCross"
	},	
	aabb = {
		enable = true,
		vertexShader = "material",
		fragmentShader = "material",
		color = {r = 1.0, g = 0.0, b = 0.0, a = 1.0}
	}
}

skybox = {
	model = "skybox",
	scale = 30.0
}

level = {
	modelName = "battleArena",
	radius = 25.0,
	position = {x = 0.0, y = -1.0, z = 0.0},
	scale = 2.0
}
player = {
	model = "barbarian",
	position = {x = 2.0, y = -1.0, z = 5.0},
	rotation = {x = 0.0, y = 0.0, z = 70.0},
	mouseSpeed = 0.5,
	eyeXOffset = 3.0, -- backwards x
	eyeYOffset = 2.0,
	targetYOffset = 2.0,
	profile = "profile1"
}

healthBar = {
	width = 1.0,
	height = 0.1,
	breadth = 0.1,
	yoffset = 2.2,
	healthLeft = 0.8,
	hitTimeDelay = 0.5, -- how much in seconds can two consecutive hits be registered
	goodColor = {r = 0.0, g = 1.0, b  = 0.0},
	badColor = {r = 1.0, g = 0.0, b  = 0.0},
	vertexShader = "material",
	fragmentShader = "material",

}
enemies = {
	models = {
		{name = "e1", modelName = "paladin", characterProfile = "profile3"},
		{name = "e2", modelName = "paladin", characterProfile = "profile3"},
		{name = "e3", modelName = "paladin", characterProfile = "profile3"},
		{name = "e4", modelName = "paladin", characterProfile = "profile2"},
		{name = "e5", modelName = "barbarian", characterProfile = "profile2"},
		{name = "e6", modelName = "barbarian", characterProfile = "profile2"},
	}
}

animation = {
	blendTime = 0.15 -- seconds
}


characterProfiles = {
	profile1 = {
		minVelocity = 1.0,
		maxRunVelocity = 15.0,
		maxWalkVelocity = 3.5,
		accelerationRun = 1.11,
		accelerationWalk = 1.05,
		animationSpeed = 2.0,
		primary = {
			modelName = "sword",
			boneAttach = "palm.01.L",
			rotation = {x = 0.0, y = 0.0, z = 70.0},
			position = {x = 0.0, y = 0.0, z = 0.05},
			IK = { -- optional
				boneEffector = "hand.L",
				ikType = "local",
				chainLength = 5,
				maxTries = 5,
				position = {x = 0.6, y = 1.3, z = 0.2},
				speed = 2.0,
				numSamples = 50,
				deltaRotation = 0.5, 
				controlPoints = {
					[1] = {x = 1.0, y = 3.0, z = 1.3},
					[2] = {x = 1.0, y = 2.0, z = 3.0},
					[3] = {x = 1.0, y = 2.0, z = 3.0},
					[4] = {x = -2.0, y = 0.0, z = 1.3}
				}
			
			}		

		},
		secondary = {
			modelName = "shield",
			boneAttach = "palm.01.R",
			rotation = {x = 0.0, y = 180.0, z = 0.0},
			position = {x = 0.0, y = 0.0, z = 0.1}
		},
	},
	profile2 = {
		minVelocity = 1.0,
		maxRunVelocity = 3.0,
		maxWalkVelocity = 1.5,
		accelerationRun = 1.05,
		accelerationWalk = 1.05,
		animationSpeed = 1.0,
		primary = {
			modelName = "sword",
			boneAttach = "palm.01.L",
			rotation = {x = 0.0, y = 0.0, z = 70.0},
			position = {x = 0.0, y = 0.0, z = 0.05},
			IK = { -- optional
				boneEffector = "hand.L",
				ikType = "local",
				chainLength = 2,
				maxTries = 2,
				position = {x = 0.6, y = 1.3, z = 0.2},
				speed = 1.2,
				numSamples = 10,
				deltaRotation = 0.2, -- euler degrees on swing how much to rotate
				controlPoints = {
					[1] = {x = 0.6, y = 1.3, z = 0.2},
					[2] = {x = 2.0, y = 1.0, z = 0.0},
					[3] = {x = 1.0, y = 6.0, z = 8.0},
					[4] = {x = 0.0, y = 0.0, z = 0.0}
				}
			
			}		

		},
		secondary = {
			modelName = "sword2",
			boneAttach = "palm.01.R",
			rotation = {x = 0.0, y = 180.0, z = 0.0},
			position = {x = 0.0, y = 0.0, z = 0.1}
		},
	},
	profile3 = {
		minVelocity = 1.0,
		maxRunVelocity = 5.0,
		maxWalkVelocity = 1.5,
		accelerationRun = 1.05,
		accelerationWalk = 1.05,
		animationSpeed = 1.5,
		primary = {
			modelName = "sword2",
			boneAttach = "palm.01.L",
			rotation = {x = 0.0, y = 0.0, z = 70.0},
			position = {x = 0.0, y = 0.0, z = 0.05},
			IK = { -- optional
				boneEffector = "hand.L",
				ikType = "local",
				chainLength = 2,
				maxTries = 2,
				position = {x = 0.6, y = 1.3, z = 0.2},
				speed = 1.2,
				numSamples = 10,
				deltaRotation = 0.2, 
				controlPoints = {
					[1] = {x = 0.6, y = 1.3, z = 0.2},
					[2] = {x = 2.0, y = 1.0, z = 0.0},
					[3] = {x = 1.0, y = 6.0, z = 8.0},
					[4] = {x = 0.0, y = 0.0, z = 0.0}
				}
			
			}		

		},
		secondary = {
			modelName = "sword2",
			boneAttach = "palm.01.R",
			rotation = {x = 0.0, y = 180.0, z = 0.0},
			position = {x = 0.0, y = 0.0, z = 0.1}
		},
	}	
}



models = {
	sword = {
		modelDir = "models/weapons/exported/sword/sword.dae",
		vertexShader = "phong",
		fragmentShader = "phong_material"
	},
	sword2 = {
		modelDir = "models/weapons/exported/sword2/sword2.dae",
		vertexShader = "phong",
		fragmentShader = "phong_material"
	},
	shield = {
		modelDir = "models/weapons/exported/shield/shield.dae",
		vertexShader = "phong",
		fragmentShader = "phong_material"
	},
	axe = {
		modelDir = "models/weapons/exported/axe/axe.dae",
		vertexShader = "phong",
		fragmentShader = "phong_material"
	},
	skybox = {
		modelDir = "models/skybox/skybox.dae",
		vertexShader = "skybox",
		fragmentShader = "skybox",	
	},
	battleArena = {
		modelDir = "models/battleArena/exported/battleArena2.dae",
		vertexShader = "skybox",
		fragmentShader = "skybox",	
	},
	gate = {
		modelDir = "models/battleArena/exported/gate.dae",
		vertexShader = "skybox",
		fragmentShader = "skybox",	
	},
	smallBall = {
		modelDir = "models/debug/smallBall/smallBall.obj",
		vertexShader = "phong",
		fragmentShader = "phong_material",
	},
	smallCross = {
		modelDir = "models/debug/smallCross/smallCross.obj",
		vertexShader = "phong",
		fragmentShader = "phong_material",
	},
	fixedAxis = {
		modelDir = "models/debug/fixedAxis/fixedAxis.obj",
		vertexShader = "phong",
		fragmentShader = "phong_material",
	},
	nanosuit  = {
		modelDir = "models/nanosuit/nanosuit.obj",
		vertexShader = "simple",
		fragmentShader = "texture_ds",
	},
	spider  = {
		modelDir = "models/assimpOBJ/spider.obj",
		vertexShader = "phong",
		fragmentShader = "texture_d",
	},
	rifle  = {
		modelDir = "models/rifle/rifle.obj",
		vertexShader = "phong",
		fragmentShader = "texture_d",
	},
	segment  = {
		modelDir = "models/rifle/segment.obj",
		vertexShader = "phong",
		fragmentShader = "phong_material",
	},
	regr01  = {
		modelDir = "models/assimpOBJ/regr01.obj",
		vertexShader = "phong",
		fragmentShader = "phong_material",
	},
	handobj  = {
		modelDir = "models/hand/hand.obj",
		vertexShader = "phong",
		fragmentShader = "phong_material",
	},
	skeleton  = {
		modelDir = "models/skeleton/human-skeleton2.3DS",
		vertexShader = "phong",
		fragmentShader = "material",
	},
	test  = {
		modelDir = "models/movingMonkey/animatedMonkey.dae",
		vertexShader = "phong",
		fragmentShader = "material",
	}

}

skinnedModels = {
	handdae  = {
		modelDir = "models/hand/hand2.dae",
		vertexShader = "skinning",
		fragmentShader = "phong_material",
		animationName = "Fist"
	},
	nielsen = {
		modelDir = "models/nielsen/nielsen.dae",
		vertexShader = "skinning",
		fragmentShader = "texture_d",
		animationName = "chickenRun",
		aabb = {
			min = {x = -0.5, y = 0.0, z = -0.1},
			max = {x = 1.5, y = 5.0, z = 1.1}
		}
	},
	barbarian = {
		modelDir = "models/barbarian/exported/barbarian7.dae",
		vertexShader = "skinning",
		fragmentShader = "texture_d",
		animationName = "wait", -- default idle animation
		additionalAnimations = {
			{animationName = "dance" , fileDir = "models/barbarian/exported/animations/dance.dae"},
			{animationName = "run" , fileDir = "models/barbarian/exported/animations/run.dae"},
			{animationName = "walk" , fileDir = "models/barbarian/exported/animations/walk.dae"},
			{animationName = "lie" , fileDir = "models/barbarian/exported/animations/lie.dae"},
			{animationName = "strafeLeft" , fileDir = "models/barbarian/exported/animations/strafeLeft.dae"},
			{animationName = "strafeRight" , fileDir = "models/barbarian/exported/animations/strafeRight.dae"},
			{animationName = "look" , fileDir = "models/barbarian/exported/animations/look.dae"},
			{animationName = "walkBackward" , fileDir = "models/barbarian/exported/animations/walkBackward.dae"},

		},
		aabb = {
			min = {x = -0.5, y = 0.0, z = -0.5},
			max = {x = 0.5, y = 2.2, z = 0.5}
		}
	},
	paladin = {
		modelDir = "models/barbarian/exported/paladin/paladin.dae",
		vertexShader = "skinning",
		fragmentShader = "texture_d",
		animationName = "wait", -- default idle animation
		additionalAnimations = {
			{animationName = "dance" , fileDir = "models/barbarian/exported/animations/dance.dae"},
			{animationName = "run" , fileDir = "models/barbarian/exported/animations/run.dae"},
			{animationName = "walk" , fileDir = "models/barbarian/exported/animations/walk.dae"},
			{animationName = "lie" , fileDir = "models/barbarian/exported/animations/lie.dae"},
			{animationName = "strafeLeft" , fileDir = "models/barbarian/exported/animations/strafeLeft.dae"},
			{animationName = "strafeRight" , fileDir = "models/barbarian/exported/animations/strafeRight.dae"},
			{animationName = "look" , fileDir = "models/barbarian/exported/animations/look.dae"},
			{animationName = "walkBackward" , fileDir = "models/barbarian/exported/animations/walkBackward.dae"},

		},
		aabb = {
			min = {x = -0.5, y = 0.0, z = -0.5},
			max = {x = 0.5, y = 2.2, z = 0.5}
		}
	},
}

vertexShaders = {
	debug = "shaders/debugVS.glsl",
	material =  "shaders/materialVS.glsl",
	simple =  "shaders/simpleVS.glsl",
	skinning = "shaders/skinningVS.glsl",
	phong_skinning = "shaders/phong_skinningVS.glsl",
	phong = "shaders/phongVS.glsl",
	skybox = "shaders/skyboxVS.glsl"
}

fragmentShaders = {
	debug = {
		shaderDir = "shaders/debugVS.glsl",
	},
	texture_ds = {
		shaderDir = "shaders/texture_ds_FS.glsl",
		samplers = {
			diffuse = {"texture_diffuse1"},
			specular = {"texture_specular1"}
		}
	},
	texture_d = {
		shaderDir = "shaders/texture_d_FS.glsl",
		samplers = {
			diffuse = {"texture_diffuse1"}
		}
	},
	material = {
		shaderDir = "shaders/materialFS.glsl"
	},
	phong_material = {
		shaderDir = "shaders/phong_materialFS.glsl"
	},
	skybox = {
		shaderDir = "shaders/skyboxFS.glsl",
		samplers = {
			diffuse = {"texture_diffuse1"}
		}
	},	

}

