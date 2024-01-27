--[[
	Shows a GUI window with info about the default example scene.
	Pauses physics simulation while the window is showing.
]]--

function init ()
	-- Create needed variables
	create(Types.Conditional, 'continueButtonPressed')
	create(Types.Conditional, 'continueButtonHovered')
	showWindow = true
	simulationActive = false
	
	-- CONTINUE button settings
	continueButtonSizeX = 350.0
	continueButtonSizeY = 64.0
	
	-- Title text shown at the top of the window, with increased font size
	titleBodyText = {}
	titleBodyText[1] = 'An example scene made out of simple assets,'
	titleBodyText[2] = 'showing different types of objects and components'
	titleBodyTextSize = 2
	
	-- Body text shown in the middle of the window
	mainBodyText = {}
	mainBodyText[1] = 'Scene contains:'
	mainBodyText[2] = 'PBR shading using Cook-Torrance BRDF'
	mainBodyText[3] = 'PBR bloom with lens dirt'
	mainBodyText[4] = 'Luminance histogram with auto exposure compensation'
	mainBodyText[5] = 'Cascaded shadow mapping with PCF filtering'
	mainBodyText[6] = 'Horizon-based ambient occlusion mapping'
	mainBodyText[7] = 'Dynamic points lights with emissive mapping'
	mainBodyText[8] = 'Sky and fog using Mie and Rayleigh atmospheric light scattering'
	mainBodyText[9] = 'HDR rendering with Uchimura tone-mapping'
	mainBodyText[10] = 'Rigid body objects for collision detection'
	mainBodyText[11] = 'Ambient sound based on day-night cycle'
	mainBodyText[12] = ''
	mainBodyText[13] = 'Controls:'
	mainBodyText[14] = '[W] - move forward'
	mainBodyText[15] = '[A] - strafe left'
	mainBodyText[16] = '[S] - move backward'
	mainBodyText[17] = '[D] - strafe right'
	mainBodyText[18] = '[C] - move upward'
	mainBodyText[19] = '[SPACE] - move downward'
	mainBodyText[20] = '[F9] - toggle fullscreen'
	mainBodyText[21] = '[F11] - toggle v-sync'
	mainBodyText[22] = '[ESC] - go back to main-menu'
	mainBodyTextSize = 22
	
	-- Pause physics simulation while this GUI window is showing
	sendData(SystemType.Physics, DataType.DataType_SimulationActive, false)
	
	-- Make sure the mouse is released, so the buttons can be pressed
	setMouseCapture(false)
		
	ErrHandlerLoc.logErrorCode(ErrorCode.Initialize_success, getLuaFilename())
end
	
function update (p_deltaTime)
	
	if showWindow then
		-- Set font
		GUI.PushFont(ImGuiFont.AboutWindow)
		
		-- Set GUI colors
		GUI.PushStyleColor(ImGuiCol.WindowBg, 0.102, 0.102, 0.102, 0.9)
		
		-- Set the GUI window to be fullscreen
		GUI.SetNextWindowPos(0, 0)
		GUI.SetNextWindowSizeFullscreen()
		
		-- BEGIN WINDOW
		GUI.Begin('##ExampleMapInfo', bitwiseOr(ImGuiWindowFlags.NoTitleBar, ImGuiWindowFlags.NoResize, ImGuiWindowFlags.NoCollapse, ImGuiWindowFlags.NoMove))
		
		screenSize = GUI.GetScreenSize()
		
		-- Increase the font size for the title text
		GUI.SetWindowFontScale(2.0)
		
		-- Draw title text
		for i = 1, titleBodyTextSize do
			GUI.TextCenterAligned(titleBodyText[i])
		end
		
		-- Go back to the regular font size
		GUI.NewLine()
		GUI.SetWindowFontScale(1.0)
		
		-- Draw main body text
		for i = 1, mainBodyTextSize do
			GUI.TextCenterAligned(mainBodyText[i])
		end
		
		-- Set the CONTINUE button colors and style
		if continueButtonHovered:isChecked() then
			GUI.PushStyleColor(ImGuiCol.Text, 0.102, 0.102, 0.102, 1.0)
		else
			GUI.PushStyleColor(ImGuiCol.Text, 0.588, 0.773, 0.29, 1.0)
		end
		GUI.PushStyleColor(ImGuiCol.Button, 0.0, 0.0, 0.0, 0.0)
		GUI.PushStyleColor(ImGuiCol.ButtonHovered, 0.588, 0.773, 0.29, 1.0)
		GUI.PushStyleVar(ImGuiStyleVar.FrameRounding, 20.0)
		GUI.PushStyleVar(ImGuiStyleVar.ButtonTextAlign, 0.5, 0.5)

		-- Increase the font size for the CONTINUE button
		GUI.SetWindowFontScale(2.0)
		
		-- Set the CONTINUE button position to be centered at the bottom of the screen
		GUI.SetCursorPosX((screenSize.x - continueButtonSizeX) * 0.5)
		GUI.SetCursorPosY(screenSize.y - continueButtonSizeY - 10.0)
		
		-- Draw the CONTINUE button
		GUI.Button('Continue', continueButtonSizeX, continueButtonSizeY, continueButtonPressed)
		GUI.IsItemHovered(continueButtonHovered)
		
		GUI.PopStyleColor(3)	-- Text, Button, ButtonHovered
		GUI.PopStyleVar(2)		-- FrameRounding, ButtonTextAlign
		GUI.SetWindowFontScale(1.0)
		
		-- END WINDOW
		GUI.End()
		
		-- Return to the previous font
		GUI.PopFont()
		
		GUI.PopStyleColor()	-- WindowBg
		
		-- If the CONTINUE button was pressed, stop showing this window
		if continueButtonPressed:isChecked() then
			showWindow = false
			simulationActive = true
			continueButtonPressed:uncheck()
			setMouseCapture(true)
			
			-- Unpause the physics simulation
			sendData(SystemType.Physics, DataType.DataType_SimulationActive, true)
		end
	end
	
end