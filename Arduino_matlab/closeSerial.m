function [] = closeSerial(comPort)
    
    clc;clear                   % clear command window and workspace
    if ~isempty(instrfind)      % if any instruments are available 
        fclose(instrfind);      % close all of them
        delete(instrfind);      % delete the object
    end
    clc;clear                   % no reason just do it!
    disp('Serial Port Closed...')   % print to command window that port closed