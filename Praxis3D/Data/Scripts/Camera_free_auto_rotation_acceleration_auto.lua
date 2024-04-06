
function init()
	-- Create needed variables
	create(Types.GameplayVariables, 'gameplayVariables');
	create(Types.InputVariables, 'inputVariables');
	create(Types.SpatialDataManager, 'spatialData');
	
	-- Create key commands, used to track pressed keys
	create(Types.KeyCommand, 'forwardKey')
	create(Types.KeyCommand, 'backwardKey')
	create(Types.KeyCommand, 'leftKey')
	create(Types.KeyCommand, 'rightKey')
	create(Types.KeyCommand, 'upKey')
	create(Types.KeyCommand, 'downKey')
	create(Types.KeyCommand, 'sprintKey')	
	create(Types.KeyCommand, 'ctrlKey')
	create(Types.KeyCommand, 'mouseLeftKey')
	create(Types.KeyCommand, 'mouseRightKey')
	create(Types.KeyCommand, 'autoRotatekey')
	
	-- Bind keys to their corresponding buttons on the keyboard
	forwardKey:bind(inputVariables.forward_key)
	backwardKey:bind(inputVariables.backward_key)
	leftKey:bind(inputVariables.left_strafe_key)
	rightKey:bind(inputVariables.right_strafe_key)
	upKey:bind(inputVariables.up_key)
	downKey:bind(inputVariables.down_key)
	sprintKey:bind(inputVariables.sprint_key)
	ctrlKey:bindByName('Key_leftctrl')
	mouseLeftKey:bindByName('Mouse_left')
	mouseRightKey:bindByName('Mouse_right')
	autoRotatekey:bindByName('Key_F4')
	
	-- Get the camera movement speed
	if cameraSpeed then 
		movementSpeedF = cameraSpeed
	else
		movementSpeedF = gameplayVariables.camera_freelook_speed
	end
	
	-- Get the camera movement speed multiplier
	if cameraSpeedMultiplier then 
		movementSpeedMultF = cameraSpeedMultiplier
	else
		movementSpeedMultF = 1.0
	end
	
	finalMovementSpeedF = movementSpeedF
	
	movementKeysActive = true
	autoRotate = false
	
	timePassed = 0.0
	timeToDelayStart = 5.0
	timeToStopSprint = timeToDelayStart + 4.4
	timeToStopMovement = timeToDelayStart + 30.0
	sprintExpired = false
	movementExpired = false
	
	ErrHandlerLoc.logErrorCode(ErrorCode.Initialize_success, getLuaFilename())
end

function update(p_deltaTime)
	
	if getPhysicsSimulationRunning() then
		timePassed = timePassed + p_deltaTime
		
		if timeToDelayStart < timePassed then
			if timeToStopSprint > timePassed then
				sprintKey:activate()
			else
				if not sprintExpired then
					sprintKey:deactivate()
					sprintExpired = true
				end
			end
			
			if timeToStopMovement > timePassed then
				backwardKey:activate()
			else
				if not movementExpired then
					backwardKey:deactivate()
					movementExpired = true
				end
			end
		end
	end
	
	if autoRotatekey:isActivated() then
		autoRotate = not autoRotate
		autoRotatekey:deactivate()
		if not autoRotate then
			rightKey:deactivate()
		end
	end
		
	if sprintKey:isActivated() then
		sprintKeyActivated = true
	end
	
	if backwardKey:isActivated() then
		backwardsKeyActivated = true
	end
	
	-- Get current mouse data
	mouseData = getMouseInfo()
	
	-- Get current spatial data and its inverse
	localTransformMat4 = spatialData:getLocalTransform()
	localTransformInverseMat4 = localTransformMat4:inverse()
	
	-- Extract position from spatial data
	positionVec3 = localTransformMat4:getPosVec3()
	
	-- Calculate new view angles based on mouse movement
	horizontalAngleF = mouseData.m_movementX * inputVariables.mouse_jaw * inputVariables.mouse_sensitivity
	verticalAngleF = mouseData.m_movementY * inputVariables.mouse_pitch * inputVariables.mouse_sensitivity
	
	-- Rotate the camera matrix by the view angles
	-- Perform rotations only if the mouse is captured inside the game window
	if getMouseCapture() or mouseRightKey:isActivated() then
		-- Rotate camera up/down based on the X direction (left/right) of the view matrix (camera's inverse matrix)
		localTransformMat4 = localTransformMat4:rotate(toRadianF(verticalAngleF), localTransformInverseMat4:getRotXVec3())
		-- Rotate camera left/right on a fixed Y direction (up/down) to not introduce any roll
		localTransformMat4 = localTransformMat4:rotate(toRadianF(horizontalAngleF), Vec3.new(0.0, 1.0, 0.0))
	end
	
	-- Perform only when in Editor mode
	if (getEngineState() == EngineStateType.Editor) then
	
		-- Stop capturing movement keys when CTRL is pressed (for example, when saving a scene with CTRL+S shortcut)
		if ctrlKey:isActivated() then
			movementKeysActive = false
		else
			movementKeysActive = true
		end
		
		-- Capture mouse when right mouse button is pressed only, since uncaptured mouse is required to interact with the Editor GUI
		if mouseRightKey:isActivated() then
			setMouseCapture(true)
		else
			setMouseCapture(false)
		end
		
	end
		
	if autoRotate then
		localTransformMat4 = localTransformMat4:rotate(toRadianF(-p_deltaTime * cameraAutoRotationSpeed), Vec3.new(0.0, 1.0, 0.0))
		rightKey:activate()
		movementKeysActive = true
	end
	
	-- Get the view direction that is facing forward
	forwardDirectionVec3 = localTransformInverseMat4:getRotZVec3()
	forwardDirectionVec3 = forwardDirectionVec3:normalize()
	
	-- Get the view direction that is facing to the right
	rightDirectionVec3 = localTransformInverseMat4:getRotXVec3()
	rightDirectionVec3 = rightDirectionVec3:normalize()
	
	-- Get the view direction that is facing up
	upDirectionVec3 = localTransformInverseMat4:getRotYVec3()
	upDirectionVec3 = upDirectionVec3:normalize()
	
	-- Increase movement speed if the sprint key is pressed
	if sprintKey:isActivated() then
		finalMovementSpeedF = finalMovementSpeedF * movementSpeedMultF
	else
		if finalMovementSpeedF > movementSpeedF then
			finalMovementSpeedF = finalMovementSpeedF / movementSpeedMultF
		end
		
		if finalMovementSpeedF < movementSpeedF then
			finalMovementSpeedF = movementSpeedF
		end
		
		if sprintExpired then
			if finalMovementSpeedF < movementSpeedF * 100000.0 then
				finalMovementSpeedF = movementSpeedF * 100000.0
			end
		end
	end
	
	-- Adjust camera position based on key presses and view direction
	-- Forwards / backwards - Z direction
	if movementKeysActive and forwardKey:isActivated() then
		positionVec3 = positionVec3 - forwardDirectionVec3:mulF(finalMovementSpeedF * p_deltaTime)
	end	
	if movementKeysActive and backwardKey:isActivated() then
		positionVec3 = positionVec3 + forwardDirectionVec3:mulF(finalMovementSpeedF * p_deltaTime)
	end
	
	-- Left / right - X direction
	if movementKeysActive and rightKey:isActivated() then
		positionVec3 = positionVec3 + rightDirectionVec3:mulF(finalMovementSpeedF * p_deltaTime)
	end	
	if movementKeysActive and leftKey:isActivated() then
		positionVec3 = positionVec3 - rightDirectionVec3:mulF(finalMovementSpeedF * p_deltaTime)
	end	
	
	-- Up / down - Y direction
	if movementKeysActive and upKey:isActivated() then
		positionVec3 = positionVec3 + upDirectionVec3:mulF(finalMovementSpeedF * 0.5 * p_deltaTime)
	end	
	if movementKeysActive and downKey:isActivated() then
		positionVec3 = positionVec3 - upDirectionVec3:mulF(finalMovementSpeedF * 0.5 * p_deltaTime)
	end
		
	-- Set the new position of the camera, and keep the W variable the same
	localTransformMat4:setPosVec4(Vec4.new(positionVec3, localTransformMat4:getPosVec4().w))
		
	-- Update the camera with the new matrix
	spatialData:setLocalTransform(localTransformMat4)
end