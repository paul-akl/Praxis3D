
function init ()
	-- Create needed variables
	create(Types.EngineVariables, 'engineVariables');
	create(Types.InputVariables, 'inputVariables');
	create(Types.WindowVariables, 'windowVariables');
	
	create(Types.Conditional, 'button1')
	create(Types.Conditional, 'checkbox1')
	button1:check()
	
	print('GUI_test.lua script initialized.')
end
	
function update (p_deltaTime)
	
	GUI.Begin('Test window')
	
	GUI.Text('SAMPLE TEXT M8', 10.0)
	
	GUI.Button('Button Test', button1)
	
	GUI.Checkbox('Checkbox Test', checkbox1)
	
	GUI.End()
		
	if button1:isChecked() then
		print('Button pressed')
		button1:uncheck()
	end
	
	if checkbox1:isChecked() then
		print('Checkbox pressed')
	end
	
	--print('test')
	
	--postChanges(Changes.GUI.Sequence)
	
	--print('test')
end