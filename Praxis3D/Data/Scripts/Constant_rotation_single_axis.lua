--[[
	Rotates an object around a single axis
	
	Inputs:
	rotationSpeed - (float) speed of rotation
	rotationAxis - (vec3f) axis of rotation
	syncWithPhysicsSimulation - (bool) stop rotating when the physics simulation is paused
]]--

function init ()
	-- Create needed variables
	create(Types.SpatialDataManager, 'spatialData');
	
	doRotation = true
	
	speed = rotationSpeed
	if speed == 0 then
		speed = 50
	end
	
	ErrHandlerLoc.logErrorCode(ErrorCode.Initialize_success, getLuaFilename())
end
	
function update (p_deltaTime)

	-- If the sync-with-physics-simulation flag is set, perform rotation only when physics simulation is running
	if syncWithPhysicsSimulation then
		doRotation = getPhysicsSimulationRunning()
	end
	
	if doRotation then
		-- Get current spatial data and its inverse
		localTransformMat4 = spatialData:getLocalTransform()
		
		-- Calculate rotation angle
		angle = (speed * p_deltaTime)
		
		-- Rotate on a given axis
		localTransformMat4 = localTransformMat4:rotate(toRadianF(angle), rotationAxis)
		
		-- Update spatial data with the new matrix
		spatialData:setLocalTransform(localTransformMat4)
	end
	
end