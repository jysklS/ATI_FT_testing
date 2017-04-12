function [s, flag] = setupSerial(comPort)
% Initialize Serial Communication between Arduino and MATLAB
% Ensure Arduino is communicating with MATLAB
% If setup complete value of setup 1 is returned instead of 0
% (Credit) - Author: Matlab Arduino; http://www.youtube.com/watch?v=ymWXCPenNM4

flag = 1;
s = serial(comPort);
set(s, 'Timeout'      , 20          );
set(s, 'ReadAsyncMode', 'continuous');
set(s, 'DataBits'     , 8           );
set(s, 'StopBits'     , 1           );
set(s, 'BaudRate'     , 9600        );
set(s, 'Parity'       , 'none'      );

fopen(s);

a = 'b';
while(a ~= 'a')
    a = fread(s, 1, 'uchar');
end

if(a == 'a')
    disp('serial Port Open!');
end

fprintf(s, '%c', 'a');
%mbox = msgbox('Serial Communication setup.'); uiwait(mbox);
fscanf(s, '%u');
end