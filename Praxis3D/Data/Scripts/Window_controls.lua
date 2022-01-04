
function init ()
	-- Create needed variables
	create(Types.EngineVariables, 'engineVariables');
	create(Types.InputVariables, 'inputVariables');
	create(Types.WindowVariables, 'windowVariables');
	
	-- Create key commands, used to track pressed keys
	create(Types.KeyCommand, 'closeKey')
	create(Types.KeyCommand, 'fullscreenKey')
	create(Types.KeyCommand, 'mouseCaptureKey')
	create(Types.KeyCommand, 'vsyncKey')
	
	-- Bind keys to their corresponding buttons on the keyboard
	closeKey:bind(inputVariables.close_window_key)
	fullscreenKey:bind(inputVariables.fullscreen_key)
	mouseCaptureKey:bind(inputVariables.clip_mouse_key)
	vsyncKey:bind(inputVariables.vsync_key)
	
	-- Get current variables
	fullscreenBool = windowVariables.fullscreen
	mouseCaptureBool = windowVariables.mouse_captured
	vsyncBool = windowVariables.vertical_sync
	
	print('Window_controls.lua script initialized.')
end

function update (p_deltaTime)
	
	if closeKey:isActivated() then
		-- Set the engine running state to false, so it is shutdown the next frame
		setEngineRunning(false)
	end	
	
	if fullscreenKey:isActivated() then
		-- Invert the corresponding bool
		fullscreenBool = not fullscreenBool
		-- Make the window fullscreen/windowed
		setFullscreen(fullscreenBool)
		-- Deactivate the button, so it's not triggered the next frame, unless pressed again
		fullscreenKey:deactivate()
	end	
	
	if mouseCaptureKey:isActivated() then
		-- Invert the corresponding bool
		mouseCaptureBool = not mouseCaptureBool
		-- Capture/release the mouse
		setMouseCapture(mouseCaptureBool)
		-- Deactivate the button, so it's not triggered the next frame, unless pressed again
		mouseCaptureKey:deactivate()
	end	
	
	if vsyncKey:isActivated() then
		-- Invert the corresponding bool
		vsyncBool = not vsyncBool
		-- Enable/disable vertical synchronization
		setVerticalSync(vsyncBool)
		-- Deactivate the button, so it's not triggered the next frame, unless pressed again
		vsyncKey:deactivate()
	end	
	
end