
function init ()
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
	create(Types.KeyCommand, 'mouseLeftKey')
	
	-- Bind keys to their corresponding buttons on the keyboard
	forwardKey:bind(inputVariables.forward_key)
	backwardKey:bind(inputVariables.backward_key)
	leftKey:bind(inputVariables.left_strafe_key)
	rightKey:bind(inputVariables.right_strafe_key)
	upKey:bind(inputVariables.up_key)
	downKey:bind(inputVariables.down_key)
	mouseLeftKey:bindByName('Mouse_left')
	
	-- Get the camera movement speed
	movementSpeedF = gameplayVariables.camera_freelook_speed
	
	print('Camera_free.lua script initialized.')
end

function update (p_deltaTime)
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
	if mouseCaptured() or mouseLeftKey:isActivated() then
		-- Rotate camera up/down based on the X direction (left/right) of the view matrix (camera's inverse matrix)
		localTransformMat4 = localTransformMat4:rotate(toRadianF(verticalAngleF), localTransformInverseMat4:getRotXVec3())
		-- Rotate camera left/right on a fixed Y direction (up/down) to not introduce any roll
		localTransformMat4 = localTransformMat4:rotate(toRadianF(horizontalAngleF), Vec3.new(0.0, 1.0, 0.0))
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
	
	-- Adjust camera position based on key presses and view direction
	-- Forwards / backwards - Z direction
	if forwardKey:isActivated() then
		positionVec3 = positionVec3 - forwardDirectionVec3:mulF(movementSpeedF * p_deltaTime)
	end	
	if backwardKey:isActivated() then
		positionVec3 = positionVec3 + forwardDirectionVec3:mulF(movementSpeedF * p_deltaTime)
	end
	
	-- Left / right - X direction
	if rightKey:isActivated() then
		positionVec3 = positionVec3 + rightDirectionVec3:mulF(movementSpeedF * p_deltaTime)
	end	
	if leftKey:isActivated() then
		positionVec3 = positionVec3 - rightDirectionVec3:mulF(movementSpeedF * p_deltaTime)
	end	
	
	-- Up / down - Y direction
	if upKey:isActivated() then
		positionVec3 = positionVec3 + upDirectionVec3:mulF(movementSpeedF * p_deltaTime)
	end	
	if downKey:isActivated() then
		positionVec3 = positionVec3 - upDirectionVec3:mulF(movementSpeedF * p_deltaTime)
	end
	
	-- Set the new position of the camera, and keep the W variable the same
	localTransformMat4:setPosVec4(Vec4.new(positionVec3, localTransformMat4:getPosVec4().w))
	
	-- Update the camera with the new matrix
	spatialData:setLocalTransform(localTransformMat4)
end