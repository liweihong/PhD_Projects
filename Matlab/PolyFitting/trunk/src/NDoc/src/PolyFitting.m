% File: PolyFitting.m
%
% This is Matlab Wrapper code for the project. 
% Click <Matlab_PolyFitting.m> for the source code.
% 
% Main functions:
%	<pushbutton5_Callback> - Line fitting button callback; recursive HT, which calls houghtTransform which calls myHT 
%	<preBtn_Callback> - Prepreocess button callback; calls preprocess or preprocess_2
%	<PolyFitting_OpeningFcn> - Starting function to initialize some global vars.
%	<rand_Callback> - Randomization button callback

function varargout = PolyFitting(varargin)
% Function: PolyFitting
% function varargout = PolyFitting(varargin)
% :
% This is entry function.
% 
%
% POLYFITTING M-file for PolyFitting.fig
%      POLYFITTING, by itself, creates a new POLYFITTING or raises the existing
%      singleton*.
%
%      H = POLYFITTING returns the handle to a new POLYFITTING or the handle to
%      the existing singleton*.
%
%      POLYFITTING('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in POLYFITTING.M with the given input arguments.
%
%      POLYFITTING('Property','Value',...) creates a new POLYFITTING or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before PolyFitting_OpeningFunction gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to PolyFitting_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help PolyFitting

% Last Modified by GUIDE v2.5 20-Dec-2006 23:02:53

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @PolyFitting_OpeningFcn, ...
                   'gui_OutputFcn',  @PolyFitting_OutputFcn, ...
                   'gui_LayoutFcn',  [] , ...
                   'gui_Callback',   []);
if nargin && ischar(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end
% End initialization code - DO NOT EDIT


function PolyFitting_OpeningFcn(hObject, eventdata, handles, varargin)
% Function: PolyFitting_OpeningFcn
% function PolyFitting_OpeningFcn(hObject, eventdata, handles, varargin)
% :
% --- Executes just before PolyFitting is made visible.
% This is starting function of GUI, initializing some global vars here.
% 
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to PolyFitting (see VARARGIN)

% Choose default command line output for PolyFitting
handles.output = hObject;

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes PolyFitting wait for user response (see UIRESUME)
% uiwait(handles.figure1);
hPoly = getappdata(0, 'hPoly');
axes(hPoly);
% initIm = uint8(255*ones(360,540)); % set image size
% setappdata(hPoly,'img',initIm);
%  x = [50  150  150  200  200  300  300  400 400 450]; % Init x and y
%  y = [250 250  200  200  250  250  200  200 250 250] ;
% setappdata(hPoly,'x',x);
% setappdata(hPoly,'y',y);
fileName = 'bitwise_Rotated_split_037.bpa_img.tiff';
initIm = imread(fileName);
setappdata(hPoly,'img',initIm);
setappdata(hPoly,'fileName',fileName)
imshow(initIm);
% line(x, y, 'LineWidth', 1, 'Color', 'r');


% --- Outputs from this function are returned to the command line.
function varargout = PolyFitting_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;



function edit1_Callback(hObject, eventdata, handles)
% hObject    handle to edit1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edit1 as text
%        str2double(get(hObject,'String')) returns contents of edit1 as a double


% --- Executes during object creation, after setting all properties.
function edit1_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on selection change in popupmenu1.
function popupmenu1_Callback(hObject, eventdata, handles)
% hObject    handle to popupmenu1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns popupmenu1 contents as cell array
%        contents{get(hObject,'Value')} returns selected item from popupmenu1


% --- Executes during object creation, after setting all properties.
function popupmenu1_CreateFcn(hObject, eventdata, handles)
% hObject    handle to popupmenu1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on selection change in popupmenu2.
function popupmenu2_Callback(hObject, eventdata, handles)
% hObject    handle to popupmenu2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns popupmenu2 contents as cell array
%        contents{get(hObject,'Value')} returns selected item from popupmenu2


% --- Executes during object creation, after setting all properties.
function popupmenu2_CreateFcn(hObject, eventdata, handles)
% hObject    handle to popupmenu2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on selection change in popupmenu3.
function popupmenu3_Callback(hObject, eventdata, handles)
% hObject    handle to popupmenu3 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns popupmenu3 contents as cell array
%        contents{get(hObject,'Value')} returns selected item from popupmenu3
hPoly = getappdata(0, 'hPoly');
initIm = getappdata(hPoly, 'img');

axes(hPoly);
[rows cols d] = size(initIm);
initIm = ones(rows, cols);
imshow(initIm);

contents = get(hObject,'String');
nEdge = str2num(contents{get(hObject,'Value')});

angle = (2*pi)/double(nEdge);
cX = cols/2;  % 400/2
cY = rows/2;  % 500/2
R = 150;   % Radium of the circle;
x = []; y = [];

for i = 0 : nEdge
    x = [x cX + R*sin(i*angle)];
    y = [y cY - R*cos(i*angle)];
end

line(x, y, 'LineWidth', 1, 'Color', 'r');
setappdata(hPoly, 'x', x);
setappdata(hPoly, 'y', y);
setappdata(hPoly, 'pdetect', []);
setappdata(hPoly, 'tetadetect', []);
setappdata(hPoly, 'img', initIm);

% --- Executes during object creation, after setting all properties.
function popupmenu3_CreateFcn(hObject, eventdata, handles)
% hObject    handle to popupmenu3 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in pushbutton2.
function pushbutton2_Callback(hObject, eventdata, handles)
% hObject    handle to pushbutton2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
hPoly = getappdata(0, 'hPoly');
initIm = getappdata(hPoly, 'img');
axes(hPoly);
[rows cols d] = size(initIm);
initIm = ones(rows, cols);
imshow(initIm);
[roi x y]=roipoly(initIm);
[rows cols d] = size(initIm);

x=x(1:length(x)-1);
y=y(1:length(y)-1);
line(x, y, 'LineWidth', 1, 'Color', 'r');
setappdata(hPoly, 'x', x);
setappdata(hPoly, 'y', y);
setappdata(hPoly, 'img', initIm);

% --- Executes during object creation, after setting all properties.
function Polygon_CreateFcn(hObject, eventdata, handles)
% hObject    handle to Polygon (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: place code in OpeningFcn to populate Polygon

setappdata(0, 'hPoly', hObject);

% --- Executes on button press in rand.
function rand_Callback(hObject, eventdata, handles)
% Function: rand_Callback
% function rand_Callback(hObject, eventdata, handles)
% :
% --- Executes on button press in rand.
% 
% Randomization function.
%
% hObject    handle to rand (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

hPoly = getappdata(0, 'hPoly');
x = getappdata(hPoly, 'x')
y = getappdata(hPoly, 'y')
initIm = getappdata(hPoly, 'img');
axes(hPoly);
[rows cols d] = size(initIm);
initIm = ones(rows, cols);

if ~(length(x)) return; end  % return, if no polygon defined.

randApp = get(handles.randPop, 'String');
randApp = randApp{get(handles.randPop,'Value')}; % get the random function;
contents = get(handles.randIntensity,'String');
INTENSITY = str2num(contents{get(handles.randIntensity,'Value')}); % get the intensity scale'
contents = get(handles.randRange,'String');
RANGE = str2num(contents{get(handles.randRange,'Value')}); % get the range scale'
if (strcmp(randApp, 'Uniform')) RANGE = RANGE * 2; end  % make it wide for uniform distribution

xRand = []; yRand = [];
%define some parameters:

% x is horizontal, y is vertical, origin is upper left corner;

for i = 2 : length(x)
    head = i-1; tail = i;
    if (y(i) > y(i-1)) %make sure start from the bottom one to upper one.
        head = i; tail = i-1;
    end
    sign = 1.0;
    if (x(head) > x(tail)) % make sure the sign of the x - coor.
        sign = -1.0;
    end
    
    if uint32(x(head)) == uint32(x(tail)) %  vertical line
        cosTheta = 0;
        sinTheta = 1;
        Len = abs(y(head) - y(tail));
    elseif uint32(y(head)) == uint32(y(tail)) %horizontal line
        cosTheta = 1;
        sinTheta = 0;
        Len = abs(x(head) - x(tail));
    else 
        tangeTheta = abs((y(head) - y(tail))/(x(head) - x(tail)));
        cosTheta = cos(atan(tangeTheta));
        sinTheta = sin(atan(tangeTheta));
        Len = abs((y(head) - y(tail))/sinTheta);
    end 
    
    if (strcmp(randApp, 'Gaussian'))
        randArray = randn(1, uint32(Len*INTENSITY + 1)); % plus 1 is for boundary case.
    else
        randArray = rand(1, uint32(Len*INTENSITY + 1)) - .5; %  plus 1 is for boundary case; uniform rand [0 1].
    end
    
    pace = 1.0/INTENSITY;
    for dis = 0 : pace : Len
        d = randArray(uint32(dis*INTENSITY)+1) * RANGE;

        if uint32(x(head)) == uint32(x(tail)) %  vertical line 
            xRand = [xRand; x(head)+d];
            yRand = [yRand; y(head)-dis];
        elseif uint32(y(head)) == uint32(y(tail)) % horizontal line
            xRand = [xRand; x(head)+sign*dis];
            yRand = [yRand; y(head)+d];
        else 
            dPrime = d * tangeTheta;
            lPrime = dis - dPrime;
            xPrime = lPrime * cosTheta;
            yPrime = lPrime * sinTheta + (d/cosTheta);
            xRand = [xRand; x(head)+sign*xPrime];
            yRand = [yRand; y(head)-yPrime];
        end
    end
end

for i = 1 : length(xRand)
    initIm(uint32(yRand(i)), uint32(xRand(i))) = 0;
end

setappdata(hPoly,'img',initIm);
setappdata(hPoly, 'pdetect', []);
setappdata(hPoly, 'tetadetect', []);
imshow(initIm);
%x = uint32(x); y = uint32(y);
%line(x, y, 'LineWidth', 1, 'Color', 'r');

% --- Executes on selection change in randPop.
function randPop_Callback(hObject, eventdata, handles)
% hObject    handle to randPop (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns randPop contents as cell array
%        contents{get(hObject,'Value')} returns selected item from randPop


% --- Executes during object creation, after setting all properties.
function randPop_CreateFcn(hObject, eventdata, handles)
% hObject    handle to randPop (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end




% --- Executes on button press in exit.
function exit_Callback(hObject, eventdata, handles)
% hObject    handle to exit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

clear all;
close all;


function randIntensity_Callback(hObject, eventdata, handles)

% --- Executes during object creation, after setting all properties.
function randIntensity_CreateFcn(hObject, eventdata, handles)
% hObject    handle to randIntensity (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end




% --- Executes on selection change in randRange.
function randRange_Callback(hObject, eventdata, handles)
% hObject    handle to randRange (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns randRange contents as cell array
%        contents{get(hObject,'Value')} returns selected item from randRange


% --- Executes during object creation, after setting all properties.
function randRange_CreateFcn(hObject, eventdata, handles)
% hObject    handle to randRange (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


function [p, t] = removeDup(pdetect, tetadetect, Accumulator)
% Function: removeDup
% function [p, t] = removeDup(pdetect, tetadetect, Accumulator)
% :
% ------- get rid of duplicated one: BUGS: 0 degree and 360 degree are
% neighbors.
% 
% Algorithm Remove multiple lines caused by noise.
% Based on radius, remove repeated lines.

p = []; t = [];
SAME = 4; % similartiy threshold. NOT GOOD IN SOME CASES
pValue = pdetect(1); tValue = tetadetect(1);
tempLargest = Accumulator(pValue, tValue);
for i = 2:length(tetadetect)
    if (tetadetect(i) - tetadetect(i-1) < SAME) && (pdetect(i) - pdetect(i-1) < SAME*2)
        if Accumulator(pdetect(i), tetadetect(i)) > tempLargest
            pValue = pdetect(i); tValue = tetadetect(i);
            tempLargest = Accumulator(pValue, tValue);
        end
    else
        p = [p; pValue]; % Add a valid line.
        t = [t; tValue];
        pValue = pdetect(i); tValue = tetadetect(i);
        tempLargest = Accumulator(pValue, tValue);
    end
end
p = [p; pValue]; % Add the last valid line.
t = [t; tValue];

function [localMax] = imLocalMax(val, thresh, kernel)
% Function: imLocalMax
% function [localMax] = imLocalMax(val, thresh, kernel)
% :
% This is the function to get the local Maxima
% val: the matrix to be evaluated.
% thresh: Threshold for the local maxima
% kernel: the kernel size for the local maxima.

[rows cols] = size(val);
localMax = zeros(rows, cols); % maxima are indicated by 1.

% get the index of the maxima of the kernel size matrix.
numR = ceil(rows/kernel); % number of runs for the kernel in the val matrix
numC = ceil(cols/kernel);
for i = 1 : numR
    if (i < numR)  % The last row run is aligned with the bottom of matrix val.
        startR = (i-1)*kernel + 1;
    else
        startR = rows - kernel;
    end
    
    for j = 1 : numC
        if (j < numC) % The last column run is aligned with the right boarder.
            startC = (j-1)*kernel + 1;
        else
            startC = cols - kernel;
        end
        
        [a b] = max(val(startR:startR+kernel, startC:startC+kernel));
        [c d] = max(a);
        if c > thresh
            localMax(startR+b(d)-1, startC + d -1) = 1; % To get the index of the local maxima
        end
    end
end

% get rid of the false maxima because of the tile partition.
[p_R, p_C] = find(localMax);
for i = 1 : length(p_R)
    startR = int32(p_R(i) - kernel/2); endR = int32(p_R(i) + kernel/2);
    startC = int32(p_C(i) - kernel/2); endC = int32(p_C(i) + kernel/2);
    startC_2 = 0; endC_2 = 0; % for degree round
    startC_3 = 0; endC_3 = 0; % for lines pointing to origin.
    if startC < 1  % 0 degree v.s. 360 degree case
        startC_2 = int32(cols + startC);  
        endC_2 = cols;
        startC = 1;
    end
    if endC > cols % 0 degree v.s. 360 degree case
        startC_2 = 1;
        endC_2 = int32(endC - cols);
        endC = cols; 
    end
    if startR < 1  % the line is approximately pointing to the origin. Therefore, 180 degree difference.
        startR = 1;  
        startC_3 = int32(mod(startC + cols/2, cols));  
        endC_3 = int32(startC_3 + kernel);
        if endC_3 > cols, endC_3 = cols; end
    end
    if endR > rows endR = rows; end

    % COMMENTS: Could the two value be equal? (very unlikely) ====== SOLVED
    if (max(max(val(startR:endR, startC:endC))) > val(p_R(i), p_C(i))) | (startC_2 ~= 0 && max(max(val(startR:endR, startC_2:endC_2))) > val(p_R(i), p_C(i))) | (startC_3 ~= 0 && max(max(val(startR:endR, startC_3:endC_3))) > val(p_R(i), p_C(i)))
        localMax(p_R(i), p_C(i)) = 0;
    elseif max(max(val(startR:endR, startC:endC))) == val(p_R(i), p_C(i)) | (startC_2 ~= 0 && max(max(val(startR:endR, startC_2:endC_2))) == val(p_R(i), p_C(i))) | (startC_3 ~= 0 && max(max(val(startR:endR, startC_3:endC_3))) == val(p_R(i), p_C(i)))
        val(p_R(i), p_C(i)) = val(p_R(i), p_C(i)) + 1; % get rid of other equal values.
    end
    
end

function [pdetect,tetadetect,Accumulator] = houghTransform(Imbinary,thresh)
% Function: houghTransform
% function [pdetect,tetadetect,Accumulator] = houghTransform(Imbinary,thresh)
% :
% Pre-conditions:
%       thresh   - a threshold value that determines the minimum number of
%                  pixels that belong to a line in image space. threshold must 
%                  be bigger than or equal to 3(default).
%		Imbinary - Binary image for HT
%
% Note: 
% Call myHT to accelerate computation. from 20s to 2s

pstep = 1;
tetastep = 1;
kernel = 32;  % 8x8 kernel for the maxima COMMENTS: could be different between Theta and distance for the kernel.

% fitting the lines to get the polygon.
p = 1:pstep:sqrt((size(Imbinary,1))^2+(size(Imbinary,2))^2);
teta = 0:tetastep:360-tetastep;
%Voting
Accumulator = zeros(length(p),length(teta));
Imbinary=uint8(Imbinary);
[imR imC] =size(Imbinary);
Accumulator = myht(Accumulator, Imbinary, p,length(p), teta, length(teta), imR, imC);
%%%%%%%%%%%%%%%%%%%%%%%%%
% [yIndex xIndex] = find(Imbinary == 0); % 0  means the data points
% for cnt = 1:size(xIndex)
%     Indteta = 0;
%     for tetai = teta*pi/180
%         Indteta = Indteta+1;
%         roi = xIndex(cnt)*cos(tetai)+yIndex(cnt)*sin(tetai);
%         if roi >= 1 & roi <= p(end)
%             temp = abs(roi-p);
%             mintemp = min(temp);
%             Indp = find(temp == mintemp);
%             Indp = Indp(1);
%             Accumulator(Indp,Indteta) = Accumulator(Indp,Indteta)+1;
%         end
%     end
% end
%%%%%%%%%%%%%%%%%%%
% Finding local maxima in Accumulator
AccumulatorbinaryMax = imLocalMax(Accumulator, thresh, kernel);
%AccumulatorbinaryMax = imregionalmax(Accumulator);
[pdetect tetadetect] = find(AccumulatorbinaryMax == 1);
% Accumulatortemp = Accumulator - thresh;
% pdetect = [];tetadetect = [];
% for cnt = 1:length(Potential_p)
%     if Accumulatortemp(Potential_p(cnt),Potential_teta(cnt)) >= 0
%         pdetect = [pdetect;Potential_p(cnt)];
%         tetadetect = [tetadetect;Potential_teta(cnt)];
%     end
% end

% Algorithm Remove multiple lines caused by noise.
% Based on radius, remove repeated lines.
%[pdetect tetadetect] = removeDup(pdetect, tetadetect, Accumulator);

% Calculation of detected lines parameters(Radius & Angle).
pdetect = pdetect * pstep;
tetadetect = tetadetect *tetastep - tetastep;


function [newIm, lineSeg] = drawLineOnData(pdetect, tetadetect, Imbinary);
% Function: drawLineOnData
% function [newIm, lineSeg] = drawLineOnData(pdetect, tetadetect, Imbinary);
% :
% --- Draw line based on Data, and remove the data which are close to lines.
% 
% The Algorithm:
% * calculate the distance from a point of the image to the line detected.
% * choose those points whose distance is less than a threshold (lambaThresh)
% * sort these points along x or y axes depending on the slope
% * choose TWO threshold, NumThresh and LenThresh, for line segmentation generation.
% * calculate how many points are falling into LenThresh, within lambaThresh distance from the line.
% * if the # is less than NumThresh, means the line ends or broken.
% 
% The Problem:
% * the points are removed after one line, so shorten another line intersect with this line.
% * to solve this problem, draw all lines together based on _original_ image, rather than _partial_ image. 

lineSeg = [];
[rows cols] = size(Imbinary);
newIm = Imbinary;
[DataY DataX] = find(Imbinary == 0); % Matlab return value is x <-> y;
for i = 1:length(pdetect)
    if (tetadetect(i) == 90)
        %line([0, 500], [pdetect(i), pdetect(i)], 'Color', 'r');
        x1 = 0;     y1 = pdetect(i);
        x2 = cols;   y2 = y1;
    else 
        ptStart = pdetect(i)/cos(pi*(tetadetect(i)/180));
        ptEnd = -rows*(tan(pi*(tetadetect(i)/180))) + ptStart;
        %line([ptStart, ptEnd], [0, 400], 'Color', 'r')
        x1 = ptStart;   y1 = 0;
        x2 = ptEnd;     y2 = rows;
    end
    
    % Equation 11
    diffX = x2 - x1;
    diffY = y2 - y1;
    disPow = diffX^2 + diffY^2;
    deSq = sqrt(disPow);
    dArray = abs(diffX*(y1-DataY) - diffY*(x1-DataX))/deSq; % Distance equation from "http://mathworld.wolfram.com/Point-LineDistance2-Dimensional.html"
    lambdaThresh = 4; % another important threshold.
    dArray = find(dArray < lambdaThresh);
    newIm(DataY(dArray), DataX(dArray)) = 1; % Remove the data fitted into a line.

    % Find the line segments between this two points.
    % Strategy Pattern for Algorithms for C++
    % Project the data on the line
    u = ((DataX(dArray) - x1)*diffX + DataY(dArray)*diffY)/disPow; % Vector
    ptAllX = x1 + u*diffX; ptAllY = y1 + u*diffY;
    
    % in case theta in 45-135 and 225-315, use X, otherwise, Y. COMMENTS:
    NumThresh = 3; % ANOTHER IMPORTANT THRESHOLD.
    LenThresh = 8; % ANOTHER IMPORTANT THRESHOLD.
    
    if (tetadetect(i) > 45 & tetadetect(i) < 135) | (tetadetect(i) > 225 & tetadetect(i) < 315)
        alongX = true;
        LenThreshDir = abs(LenThresh * sind(tetadetect(i))); % X distance based on the slope of the line.
        [ptAllX Index] = sort(ptAllX); ptAllY = ptAllY(Index); % Sort the points from min to max.
        
    else
        alongX = false;
        LenThreshDir = abs(LenThresh * cosd(tetadetect(i))); % X distance based on the slope of the line.
        [ptAllY Index] = sort(ptAllY); ptAllX = ptAllX(Index);
    end
    
    accum = 1; 
    GoodStart = false;
    while ( accum < length(ptAllX)+1)
        if alongX
            [Ind Y] = find(ptAllX > ptAllX(accum) & ptAllX <= ptAllX(accum)+LenThreshDir);
        else 
            [Ind Y] = find(ptAllY > ptAllY(accum) & ptAllY <= ptAllY(accum)+LenThreshDir);
        end
        lengthFind = length(Ind);
        if  lengthFind >= NumThresh
            if (GoodStart == false)
                GoodStart = true;
                indexStart = accum;
            end
            accum = Ind(lengthFind); % the biggest one so far.
        else
            if GoodStart == true
                GoodStart = false;
                % Find a segment.
                lineSeg = [lineSeg;ptAllX(indexStart) ptAllX(accum) ptAllY(indexStart) ptAllY(accum)];
            end
            accum = accum + 1;
        end
        
    end
    
end

% Draw Line Segmentation now
[len temp] = size(lineSeg);
for i = 1:len
    line([lineSeg(i,1), lineSeg(i,2)], [lineSeg(i,3), lineSeg(i,4)], 'Color', 'r');
end


% use c++ code with image generated as middle result
% [yIndex xIndex] = find(img == 0);
% construct the CC set with 8-connectivity
% all_cc_set = [yIndex' xIndex'];
% new_cc_set = zeros(length(yIndex), 2);
% 
% for i = 1:length(yIndex);
%     if (~ismember([yIndex(i) xIndex(i)], all_cc_set))
%     end
% end

% get rid of those CCs which are of the following characters:
% 1. CC size is small & far away from others.


% 2. CCs are inside of others.


% 3. get rid of pixels in CC which are not the edge pixles?

% --- Executes on button press in pushbutton5.
function pushbutton5_Callback(hObject, eventdata, handles)
% Function: pushbutton5_Callback
% function pushbutton5_Callback(hObject, eventdata, handles)
% :
% This is Line fitting main function, namely recursive HT.
%
% Algorithms:
%	- Load the image data from global vars.
%	- Recursive HT based on thresh (16.0) and factor (2.0).
%
% Pre-conditions:
% hObject    handle to pushbutton5 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% load the variables.
hPoly = getappdata(0, 'hPoly');
Imbinary = uint8(getappdata(hPoly, 'img'));
[rows cols] = size(Imbinary);
axes(hPoly);

if length(find(Imbinary==0)) == 0
    msgbox('Please randomize the lines!', 'Warning');
    return;
end
% Get the initilized parameters.
% thresh = 16.0; % set the initial threshold for HT.
% factor = 2.0; % set the division factor.
thresh = get(handles.initThread,'value');
factor = get(handles.IDFTag, 'value');
%stopThresh = 4.0; % set the stopping threshold for HT.

% Recursive HT.
while true
    % Hough Transform to get the lines.
    % Strategy Pattern for Algorithms for C++
    [pdetect,tetadetect,Accumulator] = houghTransform(Imbinary,thresh);

    % Draw lines for fitting
    [Imbinary, lineSeg] = drawLineOnData(pdetect, tetadetect, Imbinary);
    
    if factor == 1 break; end;
    % If all data points are fitted, just break.
    if length(find(Imbinary==0)) == 0
        break;
    end

    % adjust the threshold to smaller one.
    thresh = thresh / factor;
end

setappdata(hPoly, 'pdetect', pdetect);
setappdata(hPoly, 'tetadetect', tetadetect);




function inEdit_Callback(hObject, eventdata, handles)
% hObject    handle to inEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of inEdit as text
%        str2double(get(hObject,'String')) returns contents of inEdit as a double


% --- Executes during object creation, after setting all properties.
function inEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to inEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


function FileBrowse_Callback(hObject, eventdata, handles)
% Function: FileBrowse_Callback
% function FileBrowse_Callback(hObject, eventdata, handles)
% :
% Executes on button press in FileBrowse.
%
% Pre-conditions:
% hObject    handle to FileBrowse (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

fileName = uigetfile('*.tiff', 'Choose a image file');

hPoly = getappdata(0, 'hPoly');
if isempty(fileName) return; end;

initIm = imread(fileName);
set(handles.inEdit, 'String', fileName);
setappdata(hPoly,'fileName',fileName);
setappdata(hPoly,'img', initIm);
axes(hPoly);
imshow(initIm);




% --- Executes on slider movement.
function initThread_Callback(hObject, eventdata, handles)
% hObject    handle to initThread (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'Value') returns position of slider
%        get(hObject,'Min') and get(hObject,'Max') to determine range of slider

t = get(handles.initThread,'value');
set(handles.ITtag,'String',num2str(t,2));

% --- Executes during object creation, after setting all properties.
function initThread_CreateFcn(hObject, eventdata, handles)
% hObject    handle to initThread (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: slider controls usually have a light gray background.
if isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor',[.9 .9 .9]);
end




% --- Executes on slider movement.
function IDFTag_Callback(hObject, eventdata, handles)
% hObject    handle to IDFTag (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'Value') returns position of slider
%        get(hObject,'Min') and get(hObject,'Max') to determine range of slider

t = get(handles.IDFTag,'value');
set(handles.IDFText,'String',num2str(t,2));

% --- Executes during object creation, after setting all properties.
function IDFTag_CreateFcn(hObject, eventdata, handles)
% hObject    handle to IDFTag (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: slider controls usually have a light gray background.
if isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor',[.9 .9 .9]);
end




% --- Executes on button press in ClearBtn.
function ClearBtn_Callback(hObject, eventdata, handles)
% hObject    handle to ClearBtn (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

hPoly = getappdata(0, 'hPoly');
Imbinary = getappdata(hPoly, 'img');
initIm = data_recover(Imbinary);
setappdata(hPoly, 'img', initIm);
imshow(double(initIm));


% --- Executes on selection change in algTag.
function algTag_Callback(hObject, eventdata, handles)
% hObject    handle to algTag (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns algTag contents as cell array
%        contents{get(hObject,'Value')} returns selected item from algTag


% --- Executes during object creation, after setting all properties.
function algTag_CreateFcn(hObject, eventdata, handles)
% hObject    handle to algTag (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function preBtn_Callback(hObject, eventdata, handles)
% Function: preBtn_Callback
% function preBtn_Callback(hObject, eventdata, handles)
% :
% This is callback function for PreProcessing.
% 
% Pre-Conditions:
%	Image file name has been saved into global variable hPloy.
%
% Post-Conditions:
%	The preprocessing data has been saved, in double type. 
%
% hObject    handle to preBtn (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% load the variables.
hPoly = getappdata(0, 'hPoly');
fileName = getappdata(hPoly,'fileName');
Imbinary = uint8(imread(fileName));

[rows cols] = size(Imbinary);
axes(hPoly);
% preprocess the image to reduce noise
if get(handles.algTag, 'value') == 1
    Imbinary = preprocess(Imbinary);
else 
    Imbinary = preprocess_2(Imbinary);
end 

setappdata(hPoly, 'img', Imbinary);
imshow(double(Imbinary));

