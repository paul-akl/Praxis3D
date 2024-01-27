--[[
	Rotates an object around a two axes
	
	Inputs:
	rotationSpeed - (float) speed of rotation
	rotationAxis1 - (vec3f) first axis of rotation	
	rotationAxis2 - (vec3f) second axis of rotation
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
		
		-- Rotate on a given first axis
		localTransformMat4 = localTransformMat4:rotate(toRadianF(angle), rotationAxis1)
		
		-- Rotate on a given second axis
		localTransformMat4 = localTransformMat4:rotate(toRadianF(angle), rotationAxis2)
		
		-- Update spatial data with the new matrix
		spatialData:setLocalTransform(localTransformMat4)
	end
	
end