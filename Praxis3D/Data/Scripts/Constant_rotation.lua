
function init ()
	-- Create needed variables
	create(Types.SpatialDataManager, 'spatialData');
	
	speed = rotationSpeed
	
	if speed == 0 then
		speed = 50
	end
	
	ErrHandlerLoc.logErrorCode(ErrorCode.Initialize_success, getLuaFilename())
end
	
function update (p_deltaTime)
	-- Get current spatial data and its inverse
	localTransformMat4 = spatialData:getLocalTransform()
	
	-- Calculate rotation angle
	angle = (speed * p_deltaTime)
	
	-- Rotate left/right on a fixed Y direction (up/down) to not introduce any roll
	localTransformMat4 = localTransformMat4:rotate(toRadianF(angle), Vec3.new(0.0, 1.0, 0.0))
	
	-- Update spatial data with the new matrix
	spatialData:setLocalTransform(localTransformMat4)
end