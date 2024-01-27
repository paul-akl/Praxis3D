--[[
	Sets the volume of day and night ambient sound components
	
	Inputs:
	sunEntity 				- (string) name of the sun (directional light) component
	daySoundEntity 			- (string) name of the day time ambient sound component	
	nightSoundEntity 		- (string) name of the night time ambient sound component
	dayNightTransitionRange - (float) day-night cycle transition point denoting when the transition happens
]]--

function init ()
	-- Create needed variables
	upVector = Vec3.new(0.0, 1.0, 0.0)
	
	-- Get entity IDs of sun and sound objects
	sunEntity = getEntityID(entityNameSun)
	daySoundEntity = getEntityID(entityNameDaySound)
	nightSoundEntity = getEntityID(entityNameNightSound)
	
	ErrHandlerLoc.logErrorCode(ErrorCode.Initialize_success, getLuaFilename())
end
	
function update (p_deltaTime)
	-- Get sun spatial component
	sunSpatialComponent = getSpatialComponent(sunEntity)
	if sunSpatialComponent then
		
		-- Get sun world transform matrix
		sunWorldTransform = sunSpatialComponent:getSpatialDataChangeManager():getWorldTransform()
		
		-- Calculate day-night transition factor by first getting the dot product of sun's position and a vector pointing up
		-- and then interpolating it between a set range while clamping the result to [0 1]
		dayNightTransitionFactor = clamp(linearInterpolation(dot(sunWorldTransform:getRotZVec3(), upVector), -dayNightTransitionRange, dayNightTransitionRange), 0.0, 1.0)
		
		-- Get the day sound component and set its volume (day-night factor)
		daySoundComponent = getSoundComponentSystemObject(daySoundEntity)
		if daySoundComponent then
			sendChange(daySoundComponent, Changes.Audio.Volume, dayNightTransitionFactor)
		end
		
		-- Get the night sound component and set its volume (inverse of day-night factor)
		nightSoundComponent = getSoundComponentSystemObject(nightSoundEntity)
		if nightSoundComponent then
			sendChange(nightSoundComponent, Changes.Audio.Volume, 1.0 - dayNightTransitionFactor)
		end
		
	end
end