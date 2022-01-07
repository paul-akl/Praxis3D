
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
		
	print('GUI_test.lua script initialized.')
end

function update (p_deltaTime)
	
	GUI.Begin('Test window')
	
	GUI.Text('SAMPLE TEXT M8', 10.0)
	
	GUI.End()
	
	--print('test')
	
	--postChanges(Changes.GUI.Sequence)
	
	--print('test')
end