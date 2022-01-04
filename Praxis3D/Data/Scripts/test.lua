
print('before functions call in LUA')

function init ()
	print('init() function call in LUA')
	
	create(Types.InputVariables, 'inputVariables');
	create(Types.SpatialDataManager, 'spatialData');
	--create(Types.MouseInfo, 'mouseData');
	
	create(Types.KeyCommand, 'forwardKey')
	create(Types.KeyCommand, 'backwardKey')
	create(Types.KeyCommand, 'leftKey')
	create(Types.KeyCommand, 'rightKey')
	create(Types.KeyCommand, 'upKey')
	create(Types.KeyCommand, 'downKey')
	
	forwardKey:bind(inputVariables.forward_key)
	backwardKey:bind(inputVariables.backward_key)
	leftKey:bind(inputVariables.left_strafe_key)
	rightKey:bind(inputVariables.right_strafe_key)
	upKey:bind(inputVariables.up_key)
	downKey:bind(inputVariables.down_key)
	
	x = 0
	y = 0
	speed = 25
	
	vec1 = Vec3.new()
	vec1.x = 5
	vec1.y = 8
	
	vec2 = Vec3.new()
	vec2.x = 10
	vec2.y = 20
end

function update (p_deltaTime)

	-- Get current mouse data
	mouseData = getMouseInfo()
	
	-- Get current spatial data
	localTransformMat4 = spatialData:getLocalTransform()
	localTransformInverseMat4 = localTransformMat4:inverse()
	
	-- Extract position and rotation from spatial data
	positionVec3 = localTransformMat4:getPosVec3()
	--cameraRotationQuat = localTransformMat4:toQuat()--:normalize()
	
	-- Calculate new view angles based on mouse movement
	horizontalAngleF = mouseData.m_movementX * inputVariables.mouse_jaw * inputVariables.mouse_sensitivity
	verticalAngleF = mouseData.m_movementY * inputVariables.mouse_pitch * inputVariables.mouse_sensitivity
		
	--cameraTestQuat = cameraRotationQuat:inverse()
	
	--upDirTest = cameraTestQuat:rotateVec3(Vec3.new(0.0, 1.0, 0.0)):normalize()
	--rightDirTest = cameraTestQuat:rotateVec3(Vec3.new(1.0, 0.0, 0.0)):normalize()
	--testMatrix = localTransformMat4--Mat4.new(1.0)
	--inverseTestMatrix = localTransformMat4:inverse()
		
	--cameraRotationQuat = cameraRotationQuat:inverse()
		
	-- Turn view angles into rotation quaternions
	--yawRotateionQuat = Quat.new()
	--pitchRotationQuat = Quat.new()
	if mouseCaptured() then
		localTransformMat4 = localTransformMat4:rotate(toRadianF(verticalAngleF), localTransformInverseMat4:getRotXVec3())
		localTransformMat4 = localTransformMat4:rotate(toRadianF(horizontalAngleF), Vec3.new(0.0, 1.0, 0.0))
		--localTransformMat4 = localTransformMat4:rotate(toRadianF(horizontalAngleF), localTransformInverseMat4:getRotYVec3())
		--yawRotateionQuat = angleAxisQuat(toRadianF(horizontalAngleF), Vec3.new(0.0, 1.0, 0.0)):normalize()
		--pitchRotationQuat = angleAxisQuat(toRadianF(verticalAngleF), Vec3.new(1.0, 0.0, 0.0)):normalize()
		--yawRotateionQuat = angleAxisQuat(toRadianF(horizontalAngleF), upDirTest):normalize()
		--pitchRotationQuat = angleAxisQuat(toRadianF(verticalAngleF), rightDirTest):normalize()
	end
	
	-- Rotate the camera
	--cameraRotationQuat = cameraRotationQuat * yawRotateionQuat * pitchRotationQuat
	--cameraRotationQuat = cameraRotationQuat * pitchRotationQuat
	--cameraRotationQuat = cameraRotationQuat * yawRotateionQuat
	--cameraRotationQuat = cameraRotationQuat:normalize()
	
	-- Inverse the camera rotation to get the view diretiocn
	--viewQuat = cameraRotationQuat:inverse()
	--viewQuat = viewQuat:normalize()
	
	-- Get the view direction that is facing forward
	--forwardDirectionVec3 = viewQuat:rotateVec3(Vec3.new(0.0, 0.0, -1.0))
	forwardDirectionVec3 = localTransformInverseMat4:getRotZVec3()
	forwardDirectionVec3 = forwardDirectionVec3:normalize()
	
	-- Get the view direction that is facing to the right
	--rightDirectionVec3 = viewQuat:rotateVec3(Vec3.new(1.0, 0.0, 0.0))
	rightDirectionVec3 = localTransformInverseMat4:getRotXVec3()
	rightDirectionVec3 = rightDirectionVec3:normalize()
	
	-- Get the view direction that is facing up
	--upDirectionVec3 = viewQuat:rotateVec3(Vec3.new(0.0, 1.0, 0.0))
	upDirectionVec3 = localTransformInverseMat4:getRotYVec3()
	upDirectionVec3 = upDirectionVec3:normalize()
	
	-- Adjust camera position based on key presses and view direction
	-- Forwards / backwards - Z direction
	if forwardKey:isActivated() then
		positionVec3 = positionVec3 - forwardDirectionVec3:mulF(speed * p_deltaTime)
	end	
	if backwardKey:isActivated() then
		positionVec3 = positionVec3 + forwardDirectionVec3:mulF(speed * p_deltaTime)
	end
	
	-- Left / right - X direction
	if rightKey:isActivated() then
		positionVec3 = positionVec3 + rightDirectionVec3:mulF(speed * p_deltaTime)
	end	
	if leftKey:isActivated() then
		positionVec3 = positionVec3 - rightDirectionVec3:mulF(speed * p_deltaTime)
	end	
	
	-- Up / down - Y direction
	if upKey:isActivated() then
		positionVec3.y = positionVec3.y + (speed * p_deltaTime)
		--positionVec3 = positionVec3 + upDirectionVec3:mulF(speed * p_deltaTime)
	end	
	if downKey:isActivated() then
		positionVec3.y = positionVec3.y - (speed * p_deltaTime)
		--positionVec3 = positionVec3 - upDirectionVec3:mulF(speed * p_deltaTime)
	end
	
	positionVec4 = Vec4.new(positionVec3, localTransformMat4:getPosVec4().w)
	--positionVec4 = localTransformMat4:getPosVec4().w
	localTransformMat4:setPosVec4(Vec4.new(positionVec3, localTransformMat4:getPosVec4().w))
	--positionVec3.y = positionVec3.y + (speed * p_deltaTime)
	--x = vec1.x
	--y = vec1.y
	
	--vec1.x = x
	--vec1.y = y
	
	--vec1.x = positionVec.x
	--vec1.y = positionVec.y
	
	--vec1 = rotationVec
	--vec1:test()
	--spatialData:setWorldPosition(positionVec)
	
	--spatialData:setLocalPosition(positionVec3)
	--spatialData:setLocalRotationQuat(cameraRotationQuat)
	spatialData:setLocalTransform(localTransformMat4)
	
	print(horizontalAngleF .. ' : ' .. verticalAngleF)
	--vec1:mulVec3f(vec2)
	--print(Types.KeyCommand)
	--print('test')
	--print(positionVec3.x .. ' : ' .. positionVec3.y .. ' : ' .. positionVec3.z)
end