
function init ()
	-- Create needed variables
	create(Types.InputVariables, 'inputVariables');
	create(Types.SpatialDataManager, 'spatialData');
		
	enabled = false
	--moveSpeed = 2.0
	
	if sunMoveSpeed then
		moveSpeed = sunMoveSpeed
	else
		moveSpeed = 10.0
	end
	
	create(Types.KeyCommand, 'forwardKey')
	create(Types.KeyCommand, 'backwardKey')
	
	forwardKey:bind(inputVariables.debug_1_key)
	backwardKey:bind(inputVariables.debug_2_key)
	
	timePassed = 0.0
	timeToDelayStart = 10.0
	timeToStopSprint = timeToDelayStart + 4.0
	timeToStopFirstMovement = timeToDelayStart + 13.0
	timeToStopMovement = timeToDelayStart + 40.0
	accelerationModifier = 0.001
	maxSpeed = 12.0
	minSpeed = 4.0
	firstMovementExpired = false
	movementExpired = false
	
	ErrHandlerLoc.logErrorCode(ErrorCode.Initialize_success, getLuaFilename())
end
	
function update (p_deltaTime)
	if getPhysicsSimulationRunning() then
		timePassed = timePassed + p_deltaTime
		
		if timeToDelayStart < timePassed then
			if timeToStopSprint > timePassed then
				moveSpeed = moveSpeed * (p_deltaTime * accelerationModifier)
			else
				moveSpeed = moveSpeed / (p_deltaTime * accelerationModifier)
			end
			
			if timeToStopMovement > timePassed then
				backwardKey:activate()
			else
				if not movementExpired then
					backwardKey:deactivate()
					movementExpired = true
				end
			end
			
			if timeToStopFirstMovement < timePassed then
				firstMovementExpired = true
			end
		end
		
	end
	
	if moveSpeed > maxSpeed then
		moveSpeed = maxSpeed
	end
	
	if moveSpeed < minSpeed then
		moveSpeed = minSpeed
	end
	
	angleChanged = false
	
	-- Get current spatial data and its inverse
	localTransformMat4 = spatialData:getLocalTransform()
	localTransformInverseMat4 = localTransformMat4:inverse()
	
	spatialData:calculateLocalRotationEuler()
	eulerAngle = spatialData:getLocalSpaceData().m_spatialData.m_rotationEuler
	
	if forwardKey:isActivated() then
		eulerAngle.x = eulerAngle.x + (moveSpeed * p_deltaTime)
		eulerAngle.y = eulerAngle.y + (moveSpeed * p_deltaTime)
		angleChanged = true
	end	
	
	if backwardKey:isActivated() then
		-- Rotate camera left/right on a fixed Y direction (up/down) to not introduce any roll
		localTransformMat4 = localTransformMat4:rotate(-toRadianF(moveSpeed * p_deltaTime), Vec3.new(0.5, 0.5, 0.0))
		
		--eulerAngle.x = eulerAngle.x - (moveSpeed * p_deltaTime)
		--if not firstMovementExpired then
		--	eulerAngle.y = eulerAngle.y - (moveSpeed * p_deltaTime)
		--end
		angleChanged = true
	end	
	
	if angleChanged then
		spatialData:setLocalRotationEuler(eulerAngle)
		--spatialData:update()
		--spatialData:calculateLocalRotationQuaternion()
	end
	
	--print(eulerAngle.x)
	
	--postChanges(Changes.GUI.Sequence)
	
	-- Update the camera with the new matrix
	spatialData:setLocalTransform(localTransformMat4)
end