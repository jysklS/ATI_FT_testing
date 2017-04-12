%% Serial Message Generator Using 3D Connexion mouse for Arduino ATI_FT_testing Server
% Written by: Jameson Lee
% Last edit: 4/12/2017
% This code sets up serial communication with an arduino running
% ATI_FT_testing arduino server 'Pixhawkmock1.ino' and communication with a
% 3D Connexion space mouse. set the COM and USB ports accordingly (This was
% written for Windows 10 OS so on ubuntu these will be ttyACM0 etc..

clc;clear                               % clear workspace and command window
comPort = 'COM3';                       % Arduino COM Port
closeSerial(comPort);                   % Close any open COM channels
[s, flag] = setupSerial(comPort);       % set 's' Serial object on COM Port and report flag

mouse = vrspacemouse('USB1');           % set Space mouse and create object

% customize functionality
mouse.PositionSensitivity = 1e-3;       
mouse.LowerPositionLimit = [0 0 0];
mouse.UpperPositionLimit = [1000 1000 1000];
mouse.DominantMode = true;
mouse.LimitPosition = true;
fault = 0;                              % arming state

while 1
    xyz = round(mouse.speed([1 2 3 4 5 6])/3.5*100);                % recieve and scale mouse speed data
    xyz = 50 + [-xyz(3) -xyz(1) xyz(2) -xyz(6) -xyz(4) xyz(5)];     % offset and reorder (0-100) all points
    for i = 1:6                     % limit check for all points
        if xyz(i) > 100
            xyz(i) = 100;
        end 
        if xyz(i) < 0
            xyz(i) = 0;
        end
    end
    
    
    b = mouse.button([1 2]);        % check both space mouse buttons
    flushoutput(s);                 % clear serial buffer out
    flushinput(s);                  % clear serial buffer in
    if fault == 1                   % if armed send F/T message
        fprintf(s, '%3s', ['{',num2str(xyz(1)),',',num2str(xyz(2)),',',num2str(xyz(3)),',',num2str(xyz(4)),',',num2str(xyz(5)),',',num2str(xyz(6)),'}']);
        %fprintf('%3s', ['{',num2str(xyz(1)),',',num2str(xyz(2)),',',num2str(xyz(3)),',',num2str(xyz(4)),',',num2str(xyz(5)),',',num2str(xyz(6)),'}']);
    else                            % if disarmed send disarm message
        fprintf(s, '%3s', '###');       
        %fprintf('%3s', '###');
    end
    if b(1) == 1                    % if quit button pressed, exit while loop
        fprintf('Breaking while true due to button press\n')
        break
    end
    if b(2) == 1                    % if arm button pressed
        fprintf('\n...')            
        pause(1);                   % wait 1 second
        fprintf('\n...')
        pause(1);                   % wait... another second
        if mouse.button(2) == 1     % if arm button still pressed
            if fault == 1           % and its already armed: disarm
                fault = 0;
                fprintf('\n...Dissarmed.\n')
            else                    % else it isnt armed: so arm
                fault = 1;
                fprintf('\n...Armed!\n')
            end
        else                        % if you didn't hold the arm button nothing happends
            fprintf('\n...Hold Arming Button to Arm or Dissarm...\n')
        end
        pause(1);                   % wait another second to avoid double trip of conditional
    end
end
closeSerial(comPort);               % code end so clean up
