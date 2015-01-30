--Initial Release 0.1
-

--0.11 Before lab 3

-Added more maths functions to convert from the various assimp structures to glm (math funcs)
-Added assimp 4x4 matrix to SQTTransform decomposition (math funcs)
-Replaced the bone local transformations to be represented an SQT transform (SkinnedModel, Bone class) to allow for straightfoward intepolation of the scale, translation and rotation.
-Added global timer which can also be instantiated (Timer class)
-Added keyframe animation
-Added animation queue so we can chain together animations
--Extended the model specification in the settings file to allow for multiple animations to be imported and custom names given. This was needed because dae file allows only a single animation. Moreover that animation does not have a name. Instead, now we can import the main dae file and use extra dae files of the same model but exported with different actions from blender.
-added functions getValue and getTable alternatives to luapath library which return true/false instead of throwing an exception when no key was found. This was needed to allow for the items in the settings file to be conditional.
-removed settings for root mesh node and root bone node. Instead all the meshes and bones in the scene are imported and no transformations from the root down are ignored.

--0.2 Lab 3
-Added Inverse Kinematics using Cyclic Coordinate Descent for bones . Allows for arbitrary chain length.
-Added Bezier curves which are parametarized by time 0.0 to 1.0 and distance 0.0 to 1.0 using lookup table
-Added display visualization for curves: the line curve and the control points are visualized
-Added debug axis for the global world
-Added debug axis for the root positions of the models (local)
-Added setting options to enable/disable the various debug axis.

--0.3 Lab 4
-Fixed loading of models with multiple meshes
-Added skybox
-Added battle arena
-Rigged warrior + animated with bvh mocap for movement
-Added 3rd Person Camera
-Added weighted average animation blending for two consecutive animation
-Added primary & secondary weapon attachments, velocity & acc, directional movement(displacement, rotation and animation) to characters
-Added Ik + Curve on primary action of the character
-Added level collision detection
-Added Object to Object collision detection