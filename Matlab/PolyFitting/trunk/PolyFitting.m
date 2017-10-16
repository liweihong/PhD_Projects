% TODO:
% 1. compute the similarity of the raw range slices.
% 2. improve the boundary, especially for roof building/red pillar 
%       * watershed mehthod?
% 3. improve the chained points for DP
%       * introduce the clockwise/anti-clockwise directions.
%       % combine HT lines and ras2vec line segments.
% 4. fix the bugs of preprocessing image, such as one line (slice 63) /slice 08.
%       * fix slice 63 by set x[i] = -1, instead of 0.
%       * for slice 08, there is contradict between it and slice after 80,
%       since the noise in here could be real data in the later.
%
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
%      *See GUI Options on GUIDE Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help PolyFitting

% Last Modified by GUIDE v2.5 13-May-2007 13:22:00

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
% initIm = uint8(255*ones(360,540)); % set image size
%  x = [50  150  150  200  200  300  300  400 400 450]; % Init x and y
%  y = [250 250  200  200  250  250  200  200 250 250] ;
% setappdata(hPoly,'x',x);
% setappdata(hPoly,'y',y);

% define global variables.
fileName = 'data\new-slices\slice_1024_045.tif';
initIm = imread(fileName);
[rows, cols, notcare] = size(initIm);
if (notcare > 1) initIm = im2bw(initIm); end

setappdata(hPoly, 'fileName'                ,fileName);
setappdata(hPoly, 'curIm'                   ,initIm);
setappdata(hPoly, 'rows'                    ,rows);
setappdata(hPoly, 'cols'                    ,cols);
setappdata(hPoly, 'preFileName'             ,'pre.tif');
setappdata(hPoly, 'ras2vecFileName'         ,'ras2vec.tif');
setappdata(hPoly, 'ras2vec_output_fn'       ,'ras2vec.ply');
setappdata(hPoly, 'all_line_ply_FileName'   ,'all_lines.ply');
setappdata(hPoly, 'DP_load_fn'              ,'DP.ply');
setappdata(hPoly, 'HT_output_fn'            ,'matlab.ply'); % 
setappdata(hPoly, 'ras2vec_figure'          ,'r2v_fig'); % 
setappdata(hPoly, 'DP_figure'               ,'dp_fig'); % 

% line(x, y, 'LineWidth', 1, 'Color', 'r');

% disable some buttons
% set(handles.ras2vecBtn, 'enable', 'off');
% set(handles.pushbutton5, 'enable', 'off');
set(handles.points, 'enable', 'off');
set(handles.r2v_epsion, 'enable', 'off');
set(handles.epsilon, 'enable', 'off');
set(handles.r2vBtn, 'enable', 'off');

% show original image
image_show(initIm,hPoly,fileName, 1, 'fig');

path = 'data\clear_slices\';
setappdata(hPoly, 'choose_path', path);
fileName = sprintf('%s\\%s',pwd,fileName);
set(handles.inEdit, 'String', fileName);

% --- Outputs from this function are returned to the command line.
function varargout = PolyFitting_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;

function image_show(initIm,hPoly,title,mode,save)
[rows cols nocare] = size(initIm);
if mode
    fig = figure('Name', title, 'Units','pixels','Position',[0 0 cols rows]);
    setappdata(hPoly, save, fig);
end
imshow(initIm)

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
setappdata(hObject, 'fig', 0);


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



% --- Executes during object creation, after setting all properties.
function r2v_epsion_CreateFcn(hObject, eventdata, handles)
% hObject    handle to r2v_epsion (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: slider controls usually have a light gray background.
if isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor',[.9 .9 .9]);
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

% this is the two main threshold for HT
% pstep and tetastep: start from 1, if too large, like 20, then, it will
% converge very fast with a lot of data untouched.

pstep = 1;
tetastep = 1;
kernel = 8;  % 8x8 kernel for the maxima COMMENTS: could be different between Theta and distance for the kernel.

% fitting the lines to get the polygon.
p = 1:pstep:sqrt((size(Imbinary,1))^2+(size(Imbinary,2))^2);
teta = 0:tetastep:360-tetastep;
%Voting
Accumulator = zeros(length(p),length(teta));
[imR imC notcare] = size(Imbinary);
if notcare > 1,
   Imbinary = im2bw(Imbinary);
end
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
%AccumulatorbinaryMax = imLocalMax(Accumulator, thresh, kernel);
%AccumulatorbinaryMax = imregionalmax(Accumulator);
%[pdetect tetadetect] = find(AccumulatorbinaryMax == 1);

[a b]=max(Accumulator);
[c d]=max(a);
pdetect = b(d);
tetadetect= d;
%[pdetect tetadetect] = find(Accumulator > thresh);

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


function [newIm, lineSeg] = drawLineOnData(pdetect, tetadetect, Imbinary, count);
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
% * to solve this problem, draw all lines together based on _original_
% image, rather than _partial_ image. 

lineSeg = [];
[rows cols] = size(Imbinary);
newIm = Imbinary;
[DataY DataX] = find(Imbinary == 0); % Matlab return value is x <-> y;
color = 'r';
if count > 0  color = 'g'; end
if count > 1  color = 'b'; end
if count > 2  color = 'c'; end
if count > 3  color = 'k'; end
 
for i = 1:length(pdetect)
    if (tetadetect(i) == 90)
        %line([0, 1024], [pdetect(i), pdetect(i)], 'Color', color);
        x1 = 0;     y1 = pdetect(i);
        x2 = cols;   y2 = y1;
    else 
        ptStart = pdetect(i)/cos(pi*(tetadetect(i)/180));
        ptEnd = -rows*(tan(pi*(tetadetect(i)/180))) + ptStart;
        %line([ptStart, ptEnd], [0, 400], 'Color', color)
        x1 = ptStart;   y1 = 0;
        x2 = ptEnd;     y2 = rows;
    end
    
    % Equation 11
    diffX = x2 - x1;
    diffY = y2 - y1;
    disPow = diffX^2 + diffY^2;
    deSq = sqrt(disPow);
    dArray = abs(diffX*(y1-DataY) - diffY*(x1-DataX))/deSq; % Distance equation from "http://mathworld.wolfram.com/Point-LineDistance2-Dimensional.html"
    lambdaThresh = 2; % another important threshold. tried, 1, 2, 4, the best one is 2.
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

%figure; imshow((newIm));
%figure; imshow((newIm));
% Draw Line Segmentation now
[len temp] = size(lineSeg);
length(ptAllX)
lineSeg
for i = 1:len
    line([lineSeg(i,1), lineSeg(i,2)], [lineSeg(i,3), lineSeg(i,4)], 'Color', 'r');
end



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

hPoly = getappdata(0, 'hPoly');
path = getappdata(hPoly, 'choose_path');
if length(path)==0, path = ''; end;
[fileName, pathName] = uigetfile('*.tif; *.tiff; *.bmp', 'Choose a image file', path);

if fileName==0 return; end;

setappdata(hPoly, 'choose_path', pathName);

fileName = [pathName fileName];
initIm = imread(fileName);
[rows cols notcare] = size(initIm);
if (notcare > 1) initIm = im2bw(initIm); end;
set(handles.inEdit, 'String', fileName);
setappdata(hPoly, 'fileName',fileName);
setappdata(hPoly, 'curIm', initIm);
%image(initIm);
image_show(initIm, hPoly, fileName, 1, 'none');

% disable some buttons
% set(handles.ras2vecBtn, 'enable', 'off');
% set(handles.pushbutton5, 'enable', 'off');
set(handles.points, 'enable', 'off');
set(handles.r2v_epsion, 'enable', 'off');
set(handles.epsilon, 'enable', 'off');
set(handles.r2vBtn, 'enable', 'off');


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

% --- Executes during object creation, after setting all properties.
function mask_CreateFcn(hObject, eventdata, handles)
% hObject    handle to mask (see GCBO)
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

% --- Executes when figure1 is resized.
function figure1_ResizeFcn(hObject, eventdata, handles)
% hObject    handle to figure1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)



% --- Executes during object creation, after setting all properties.
function epsilon_CreateFcn(hObject, eventdata, handles)
% hObject    handle to epsilon (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: slider controls usually have a light gray background.
if isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor',[.9 .9 .9]);
end




% --- Executes on slider movement.
function FinTH_Callback(hObject, eventdata, handles)
% hObject    handle to FinTH (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'Value') returns position of slider
%        get(hObject,'Min') and get(hObject,'Max') to determine range of slider
t = get(handles.FinTH,'value');
set(handles.FTtag,'String',num2str(t,2));


% --- Executes during object creation, after setting all properties.
function FinTH_CreateFcn(hObject, eventdata, handles)
% hObject    handle to FinTH (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: slider controls usually have a light gray background.
if isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor',[.9 .9 .9]);
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

msg_handler = msgbox('Please waiting....', 'working....');

% load the variables.
hPoly = getappdata(0, 'hPoly');
fileName = getappdata(hPoly,'fileName');
tempfn = getappdata(hPoly,'preFileName');
vecFile = getappdata(hPoly,'ras2vecFileName');

% preprocess the image to reduce noise
    
%     if get(handles.algTag, 'value') == 1
%         system(sprintf('src\\project.exe -P %s -o %s > temp', fileName, tempfn));
%     else 
%         system(sprintf('src\\project.exe -p %s -o %s > temp', fileName, tempfn));
%     end 

    system(sprintf('src\\project.exe -3 %s -o %s > temp', fileName, tempfn));

% if get(handles.algTag, 'value') == 1
%     Imbinary = preprocess(Imbinary);
% else 
%     Imbinary = preprocess_2(Imbinary);
% end 

%setappdata(hPoly, 'img', Imbinary);
preImg = imread(tempfn);
ras2vecIm = im2bw(preImg);
imwrite(ras2vecIm, vecFile, 'tif');
figure(getappdata(hPoly,'fig'));
image_show(preImg, hPoly, tempfn, 0, 'fig');
setappdata(hPoly, 'curIm', ras2vecIm);

% enable some buttons
set(handles.ras2vecBtn, 'enable', 'on');
set(handles.pushbutton5, 'enable', 'on');

close(msg_handler);

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

msg_handler = msgbox('Please waiting....', 'working....');

% load the variables.
hPoly = getappdata(0, 'hPoly');
fn = getappdata(hPoly, 'preFileName');
%Imbinary = uint8(imread(fn));
Imbinary = getappdata(hPoly, 'curIm');
[rows cols nocare] = size(Imbinary);
if (nocare > 1) Imbinary = im2bw(Imbinary); end;
image_show(Imbinary, hPoly, 'Hough transform', 1, 'Hough_transform');
if length(find(Imbinary==0)) == 0
    msgbox('Please randomize the lines!', 'Warning');
    return;
end
% Get the initilized parameters.
% thresh = 16.0; % set the initial threshold for HT.
% factor = 2.0; % set the division factor.
thresh = get(handles.initThread,'value');
end_thresh = get(handles.FinTH, 'value');
factor = get(handles.IDFTag, 'value');
%stopThresh = 4.0; % set the stopping threshold for HT.

all_line_seg = [];
% Recursive HT.
count=0;
while true
    % Hough Transform to get the lines.
    % Strategy Pattern for Algorithms for C++
    [pdetect,tetadetect,Accumulator] = houghTransform(Imbinary,thresh);

    % Draw lines for fitting
    tempIm = Imbinary;
    [Imbinary, lineSeg] = drawLineOnData(pdetect, tetadetect, Imbinary, count);
    all_line_seg = [all_line_seg;lineSeg];
    
    if factor == 1 break; end;
    % If all data points are fitted, just break.
    if length(find(Imbinary==0)) == 0
        break;
    else disp(sprintf('image size %d, p: %d, t:%d\n', length(find(Imbinary==0)), pdetect(1), tetadetect(1)));
    end

    % adjust the threshold to smaller one.
    thresh = thresh / factor;
    count = count+1;
    
    % convergence
    diffIm = tempIm == Imbinary;
    if length(find(diffIm == 0)) == 0
        break;
    end
    
    %if thresh < end_thresh, break; end;
end

setappdata(hPoly, 'pdetect', pdetect);
setappdata(hPoly, 'tetadetect', tetadetect);
setappdata(hPoly, 'all_lines', all_line_seg);

set(handles.points, 'enable', 'on');
close(msg_handler);

% --- Executes on button press in duplicate.
function duplicate_Callback(hObject, eventdata, handles)
% hObject    handle to duplicate (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

hPoly = getappdata(0, 'hPoly');
fn = getappdata(hPoly, 'preFileName');
im = imread(fn);
setappdata(hPoly, 'fig', 0);
image_show(im, hPoly, fn, 1, 'none');


% --- Executes on button press in points.
function points_Callback(hObject, eventdata, handles)
% hObject    handle to points (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

msg_handler = msgbox('Please waiting....', 'working....');
% preprocessed image
hPoly = getappdata(0, 'hPoly');
fn = getappdata(hPoly, 'preFileName');
all_line_ply_fn = getappdata(hPoly, 'all_line_ply_FileName');

im_pre = imread(fn);
[rows cols notcare] = size(im_pre);
im = ones(rows,cols);
%new_image = im2bw(im_pre);
new_image = im_pre;

hPoly = getappdata(0, 'hPoly');
HT_output_fn = getappdata(hPoly, 'HT_output_fn');
DIST_MASK = get(handles.mask,'value');  % default : 16; % distance mask

all_lines = getappdata(hPoly, 'all_lines'); 
all_lines = int32(all_lines);
write_to_file(all_line_ply_fn, all_lines);
% line: x1, x2, y1, y2

% draw the new image (blank)
fig = getappdata(hPoly, 'DP_figure');
image_show(im, hPoly, 'DP algorithm with Hough Transform', 1, fig);

all_points = get_points_for_DP(all_lines, DIST_MASK, new_image);

x1 = all_points(1,1); y1 = all_points(1,2);
% line(all_points(:,1), all_points(:,2), 'Color', 'r');
for i = 2 : length(all_points(:,1))
    x2 = all_points(i,1); y2 = all_points(i,2);
    if mod(i,2) == 0,
        co = 'r';
    else co = 'b';
    end
    line([x1 x2],[y1 y2],'Color', co);
    x1 = x2; y1 = y2;
end
% write to file for DP algorithm.
write_to_file(HT_output_fn, all_points);

% enable the epsilon slider
set(handles.epsilon, 'enable', 'on');

close(msg_handler);

function write_to_file(fn, all_points)
fid = fopen(fn,'w');
[rows, cols, notcard] = size(all_points);
for i = 1:rows,
    for j = 1:cols
        fprintf(fid, '%d ', all_points(i,j));
    end
    fprintf(fid,'\n');
end
fclose(fid);

function [mid_points, new_image] = get_middle_image_data(pre_point, cur_point, old_image)
% find image data between two points
new_image = old_image;
[rows, cols, notcare] = size(new_image);
mid_points = [];
x1 = pre_point(1);
y1 = pre_point(2);
x2 = cur_point(1);
y2 = cur_point(2);
manhan_x = abs(x1-x2);
manhan_y = abs(y1-y2);
max_dis = manhan_x + manhan_y;
if max_dis < 3, return; end

x = x1; y = y1;
run = 0;
new_x = 0; new_y = 0;
while true

    run = run + 1;

    temp_x = x - run;
    if temp_x > 0
        for j = y - run: y + run
            if temp_x == x2 & j == y2, return; end
            if j > 0 & j < rows + 1
                if new_image(j, temp_x) == 0,
                    temp_dis = abs(temp_x - x2) + abs(j - y2);
                    if temp_dis < max_dis
                        max_dis = temp_dis;
                        new_x = temp_x;
                        new_y = j;
                    end
                end
            end
        end
    end

    temp_x = x + run;
    if temp_x < cols + 1
        for j = y - run: y + run
            if temp_x == x2 & j == y2, return; end
            if j > 0 & j < rows + 1
                if new_image(j, temp_x) == 0,
                    temp_dis = abs(temp_x - x2) + abs(j - y2);
                    if temp_dis < max_dis
                        max_dis = temp_dis;
                        new_x = temp_x;
                        new_y = j;
                    end
                end
            end
        end
    end

    temp_y = y - run;
    if temp_y > 0
        for j = x - run: x + run
            if temp_y == y2 & j == x2, return; end
            if j > 0 & j < cols + 1
                if new_image(temp_y, j) == 0,
                    temp_dis = abs(temp_y - y2) + abs(j - x2);
                    if temp_dis < max_dis
                        max_dis = temp_dis;
                        new_x = j;
                        new_y = temp_y;
                    end
                end
            end
        end
    end

    temp_y = y + run;
    if temp_y < rows + 1
        for j = x - run: x + run
            if temp_y == y2 & j == x2, return; end
            if j > 0 & j < cols + 1
                if new_image(temp_y, j) == 0,
                    temp_dis = abs(temp_y - y2) + abs(j - x2);
                    if temp_dis < max_dis
                        max_dis = temp_dis;
                        new_x = j;
                        new_y = temp_y;
                    end
                end
            end
        end
    end

    % update x, y, mid_points, run, new_x, new_y
    if (new_x > 0)
        if (new_y == 0), disp('ERROR:!!!!!!!!!!!!!!!!!!!!!!!!!!!!'); end;
        x = new_x; y = new_y;
        mid_points = [x y; mid_points];
        new_image(y,x) = 1;
        run = 0;
        new_x = 0; new_y = 0;
    end
    
end

% check if there is an conflict.
function [bool] = check_intersection(image, cur_point, new_point)
bool = 0;
return;
[rows cols notcare] = size(image);
line_points = comp_points(cur_point, new_point, rows, cols);
all_coords = (line_points(:,1)-1)*rows + line_points(:,2);
intersection = find(image(all_coords) == 0);
cur_coord = (cur_point(1)-1)*rows + cur_point(2);
tt = length(find(image==0));
if length(setdiff(all_coords(intersection), cur_coord)) > 0, 
    bool=1; 
end


function [image_data, new_image] = find_image_data(cur_point, DIST_MASK, old_image, processed_image)
% find CC and image_data around cur_point inside of DIST_MASK
% assume only ONE Connected Component existed.

[rows cols notcare] = size(old_image);
image_data = [];
new_image = old_image;
min_x = max(1, cur_point(1) - DIST_MASK);
max_x = min(cols, cur_point(1) + DIST_MASK);
min_y = max(1, cur_point(2) - DIST_MASK);
max_y = min(rows, cur_point(2) + DIST_MASK);

% find the first image_data
for i = 1:DIST_MASK
    temp_x = cur_point(1) - i;
    if temp_x > 0
        for j = cur_point(2) - i: cur_point(2) + i
            if j > 0 & j < rows + 1
                if new_image(j, temp_x) == 0 & check_intersection(processed_image, cur_point, [temp_x, j]) == 0,
                    new_image(j,temp_x) = 1;
                    image_data = [temp_x, j];
                    break;
                end
            end
        end
        if length(image_data) > 0, break; end
    end
    
    temp_x = cur_point(1) + i;
    if temp_x < cols + 1
        for j = cur_point(2) - i: cur_point(2) + i
            if j > 0 & j < rows + 1
                if new_image(j, temp_x) == 0 & check_intersection(processed_image, cur_point, [temp_x, j]) == 0,
                    new_image(j,temp_x) = 1;
                    image_data = [temp_x, j];
                    break;
                end
            end
        end
        if length(image_data) > 0, break; end
    end
    
    temp_y = cur_point(2) - i;
    if temp_y > 0
        for j = cur_point(1) - i: cur_point(1) + i
            if j > 0 & j < cols + 1
                if new_image(temp_y, j) == 0 & check_intersection(processed_image, cur_point, [j, temp_y]) == 0,
                    new_image(temp_y, j) = 1;
                    image_data = [j, temp_y];
                    break;
                end
            end
        end
        if length(image_data) > 0, break; end
    end
    
    temp_y = cur_point(2) + i;
    if temp_y < rows + 1
        for j = cur_point(1) - i: cur_point(1) + i
            if j > 0 & j < cols + 1
                if new_image(temp_y, j) == 0 & check_intersection(processed_image, cur_point, [j, temp_y]) == 0,
                    new_image(temp_y, j) = 1;
                    image_data = [j, temp_y];
                    break;
                end
            end
        end
        if length(image_data) > 0, break; end
    end
end

% no image data available
if length(image_data) == 0, return; end

x = image_data(1);
y = image_data(2);
% find the CC
while true
    if (x+1) > max_x | (y+1) > max_y | (x-1) < min_x | (y-1) < min_y,
        break;
    end
    
    if new_image(y, x+1) == 0,
        x = x+1;
        new_image(y, x) = 1;
        image_data = [x, y; image_data];
        continue;
    end
    
    if new_image(y, x-1) == 0,
        x = x-1;
        new_image(y, x) = 1;
        image_data = [x, y; image_data];
        continue;
    end
    
    if new_image(y+1, x) == 0,
        y = y+1;
        new_image(y, x) = 1;
        image_data = [x, y; image_data];
        continue;
    end
    
    if new_image(y-1, x) == 0,
        y = y-1;
        new_image(y, x) = 1;
        image_data = [x, y; image_data];
        continue;
    end

    if new_image(y-1, x-1) == 0,
        y = y-1;
        x = x-1;
        new_image(y, x) = 1;
        image_data = [x, y; image_data];
        continue;
    end    

    if new_image(y-1, x+1) == 0,
        y = y-1;
        x = x+1;
        new_image(y, x) = 1;
        image_data = [x, y; image_data];
        continue;
    end    

    if new_image(y+1, x-1) == 0,
        y = y+1;
        x = x-1;
        new_image(y, x) = 1;
        image_data = [x, y; image_data];
        continue;
    end    

    if new_image(y+1, x+1) == 0,
        y = y+1;
        x = x+1;
        new_image(y, x) = 1;
        image_data = [x, y; image_data];
        continue;
    end    
    
    % if no data found, just quit.
    break;
end

function [points] = comp_points(p1, p2, rows, cols)
points = [];
x1 = double(p1(1,1));
x2 = double(p2(1,1));
y1 = double(p1(1,2));
y2 = double(p2(1,2));
if x1 == x2
    min_y = int32(min(y1, y2));
    max_y = int32(max(y1, y2));
    len = int32(max_y - min_y);
    a = int32(x1*ones(len+1,1));
    if (len==0), a = int32(x1); end
    b = int32([min_y:max_y]');
    points = [a b];
else
    m = (y2 - y1)/(x2 - x1);
    
    if abs(m) < 1, % compute on X - axis, otherwise y axis.
        sign_x = -(x1-x2)/abs(x1-x2); %sign must be minus
        for i = x1:sign_x:x2
            temp_y = int32(y1 + (i-x1)*m);
            if temp_y < 1 | temp_y > rows, break; end
            points = [int32(i) temp_y; points];
        end
    else
        sign_y = -(y1-y2)/abs(y1-y2);
        m = 1/m;  % get the inverse of m.
        for i = y1:sign_y:y2
            temp_x = int32(x1 + (i-y1)*m);
            if temp_x < 1 | temp_x > cols, break; end;
            points = [temp_x int32(i); points];
        end
    end
end

function [new_image, p_img] = remove_data_around_line(old_image, new_line, pro_image)
% remove all image data around the new_line on new_image
new_image = old_image;
p_img = pro_image;
[rows, cols, notcare]=size(new_image);
mask = 2;
mask_p = 0;
x1 = double(new_line(1,1));
x2 = double(new_line(2,1));
y1 = double(new_line(1,2));
y2 = double(new_line(2,2));

if x1 == x2
    min_y = min(y1, y2);
    max_y = max(y1, y2);
    for i = min_y : max_y,
        new_image(max(1,int32(i-mask)):min(rows,int32(i+mask)), max(1,int32(x1-mask)):min(cols,int32(x1+mask))) = 1;
        p_img(max(1,int32(i-mask_p)):min(rows,int32(i+mask_p)), max(1,int32(x1-mask_p)):min(cols,int32(x1+mask_p))) = 0;
        %new_image(i, x1) = 0;
    end
else
    m = (y2 - y1)/(x2 - x1);
    
    if abs(m) < 1, % compute on X - axis, otherwise y axis.
        sign_x = -(x1-x2)/abs(x1-x2); %sign must be minus
        for i = x1:sign_x:x2
            temp_y = int32(y1 + (i-x1)*m);
            new_image(max(1,temp_y-mask):min(rows,temp_y+mask), max(1,i-mask):min(cols,i+mask)) = 1;
            p_img(max(1,temp_y-mask_p):min(rows,temp_y+mask_p), max(1,i-mask_p):min(cols,i+mask_p)) = 0;
            %new_image(temp_y, i) = 0;
        end
    else
        sign_y = -(y1-y2)/abs(y1-y2);
        m = 1/m;  % get the inverse of m.
        for i = y1:sign_y:y2
            temp_x = int32(x1 + (i-y1)*m);
            new_image(max(1,i-mask):min(rows,i+mask), max(1,temp_x-mask):min(cols,temp_x+mask)) = 1;
            p_img(max(1,i-mask_p):min(rows,i+mask_p), max(1,temp_x-mask_p):min(cols,temp_x+mask_p)) = 0;
            %new_image(i, temp_x) = 0;
        end
    end
end


function [new_image] = remove_data_around_line_1(old_image, new_line)
% remove all image data around the new_line on new_image
new_image = old_image;
x1 = double(new_line(1,1));
x2 = double(new_line(2,1));
y1 = double(new_line(1,2));
y2 = double(new_line(2,2));
new_image(int32(y1), int32(x1)) = 1; % clear the two end points
new_image(int32(y2), int32(x2)) = 1;

[DataY DataX] = find(new_image == 0); % Matlab return value is x <-> y;
diffX = (x2 - x1);
diffY = (y2 - y1);
disPow = diffX^2 + diffY^2;
deSq = sqrt(double(disPow));
dArray = abs(diffX*(y1-DataY) - diffY*(x1-DataX))/deSq; % Distance equation from "http://mathworld.wolfram.com/Point-LineDistance2-Dimensional.html"
lambdaThresh = 2; % another important threshold. tried, 1, 2, 4, the best one is 2.
dArray = find(dArray < lambdaThresh);


 %AllX = DataX(dArray);
 %dArray = find(AllX > min(x1,x2) & AllX < max(x1,x2));
new_image(DataY(dArray), DataX(dArray)) = 1; % Remove the data fitted into a line.

% for i = 1: length(dArray)
%     if DataX(dArray) > min(x1,x2) & DataX(dArray) < max(x1,x2)
%         new_image(DataY(dArray), DataX(dArray)) = 1;
%     end
% end

function [p] = cal_intersection(line1, line2)
% compute the intersection point
p = ones(1,2);
if line1(1,1) == line1(2,1) | line2(1,1) == line2(2,1),
    if line1(1,1) == line1(2,1)
        m1 = (line2(1,2) - line2(2,2)) / (line2(1,1) - line2(2,1));
        b1 = line2(1,2) - m1*line2(1,1);
        p(1) = line1(1,1);
        p(2) = m1*p(1) + b1;
    else
        m1 = (line1(1,2) - line1(2,2)) / (line1(1,1) - line1(2,1));
        b1 = line1(1,2) - m1*line1(1,1);
        p(1) = line2(1,1);
        p(2) = m1*p(1) + b1;
       
    end
    return;
end
m1 = (line1(1,2) - line1(2,2)) / (line1(1,1) - line1(2,1));
m2 = (line2(1,2) - line2(2,2)) / (line2(1,1) - line2(2,1));
b1 = line1(1,2) - m1*line1(1,1);
b2 = line2(1,2) - m2*line2(1,1);
p(1) = -(b2-b1)/(m2-m1);
p(2) = m1*p(1) + b1;

function [boolean] = check_line_parallel(line1, line2)
% check the line paraelle
if (line1(1,1) == line1(2,1)) & (line2(1,1) == line2(2,1)),
    boolean = true;
    return;
end

if (line1(1,1) == line1(2,1)) | (line2(1,1) == line2(2,1)),
    boolean = false;
    return;
end

boolean = false;
m1 = (line1(1,2) - line1(2,2)) / (line1(1,1) - line1(2,1));
m2 = (line2(1,2) - line2(2,2)) / (line2(1,1) - line2(2,1));

if (m1 == m2)
    boolean = true;
end

function [p] = reverse_vector(old)
% assume 2D
[rows cols] = size(old);
p = ones(rows, cols);
for i = 1: rows,
    p(i,:) = old(rows-i+1, :);
end

function [lines] = find_lines(cur_point, all_lines, DIST_MASK, lines_included, processed_image)
% Function: find_lines
% function find_lines(cur_point, all_lines, DIST_MASK)
%
% This function is used to find all lines close to the cur_points
%
% Algorithms:
%	- all_lines is of [x1 x2 y1 y2]
%	- check [x1 y1] [x2 y2] falling inside of cur_point + DIST_MASK
%
% Pre-conditions:
% cur_point - current point as a reference
% all_lines - all lines detected by HT
% DIST_MASK - distance mask

min_x = cur_point(1) - DIST_MASK;
max_x = cur_point(1) + DIST_MASK;
min_y = cur_point(2) - DIST_MASK;
max_y = cur_point(2) + DIST_MASK;
[total_lines notcare] = size(all_lines);
lines = [];
for i = 1: total_lines
    if ismember(i, lines_included)
        continue;
    end
    x1 = all_lines(i,1); y1 = all_lines(i,3);
    x2 = all_lines(i,2); y2 = all_lines(i,4);
    if ((x1 > min_x) & (x1 < max_x) & (y1 > min_y) & (y1 < max_y)) | ((x2 > min_x) & (x2 < max_x) & (y2 > min_y) & (y2 < max_y))
        if (check_intersection(processed_image, cur_point, [x1 y1]) == 0 & check_intersection(processed_image, cur_point, [x2 y2]) == 0)
            lines = [lines i];
        end
    end
end



% --- Executes on slider movement.
function epsilon_Callback(hObject, eventdata, handles)
% hObject    handle to epsilon (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'Value') returns position of slider
%        get(hObject,'Min') and get(hObject,'Max') to determine range of slider

t = get(handles.epsilon,'value');
set(handles.epsilonTag,'String',num2str(t,2));

% extra load
hPoly = getappdata(0, 'hPoly');
rows = getappdata(hPoly, 'rows');
cols = getappdata(hPoly, 'cols');
dp_load_fn = getappdata(hPoly, 'DP_load_fn');
HT_output_fn = getappdata(hPoly, 'HT_output_fn');

% call project.exe to invoke DP algorithm
param = sprintf('-e %f', t);
command = sprintf('src\\project.exe  %s -d %s > temp', param, HT_output_fn);
system(command);

im = ones(rows,cols);
fig = getappdata(hPoly, getappdata(hPoly, 'DP_figure'));
figure(fig);
show_epsilon_image(dp_load_fn, im, 0, 'DP algorithm with Hough Transform', 0, fig);

function show_epsilon_image(fn, tt, mode, name, new, save)
[rows cols nocare] = size(tt);
tt = im2bw(tt);
hPoly = getappdata(0, 'hPoly');
image_show(tt, hPoly, name, new, save);

fid = fopen(fn,'r');
X=[]; Y=[];
co = 'r';
i=1;
while 1
    tline = fgetl(fid);
    if ~ischar(tline), break, end
    if (strncmp(tline, 'POLY', 4))
       continue;
    end
	if (strncmp(tline, 'END', 3))
        i = 1;
% 		line(X,Y,'Color', co);
		X=[]; Y=[];
        if (strcmp(co,'r')) 
            co = 'b';
        else co = 'r';
        end
		continue;
	end
	
    A = sscanf(tline, '%d', 2);
    X = [A(1)+1, X];
    if mode,
        Y = [392-A(2), Y];
    else Y = [A(2), Y];
    end
    
    if (i>1)
        X1 = [X(2) X(1)];
        Y1 = [Y(2) Y(1)];
        line(X1,Y1,'Color', co);
        if (strcmp(co,'r')) 
            co = 'b';
        else co = 'r';
        end
    end
    
    i=i+1;
end
fclose(fid);

% --- Executes on button press in ras2vecBtn.
function ras2vecBtn_Callback(hObject, eventdata, handles)
% hObject    handle to ras2vecBtn (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

hPoly = getappdata(0, 'hPoly');
ras2vec_output_fn = getappdata(hPoly, 'ras2vec_output_fn');
fileName = getappdata(hPoly, 'ras2vecFileName');
fig = getappdata(hPoly, 'ras2vec_figure');

command = sprintf('ras2vec.exe  -p -e 10 %s > temp', fileName);
system(command);

tt = imread(fileName);
[rows, cols, notcare] = size(tt);
im = ones(rows,cols);
fig = getappdata(hPoly, 'ras2vec_figure');
ras2vec_output_fn='C:\Liwh\Dev\Matlab\PolyFitting\src\slice_082.ply';
show_epsilon_image(ras2vec_output_fn, im, 1, 'ras2vec', 1, fig);

set(handles.r2v_epsion, 'enable', 'on');
set(handles.r2vBtn, 'enable', 'on');

% --- Executes on slider movement.
function r2v_epsion_Callback(hObject, eventdata, handles)
% hObject    handle to r2v_epsion (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'Value') returns position of slider
%        get(hObject,'Min') and get(hObject,'Max') to determine range of slider

t = get(handles.r2v_epsion,'value');
set(handles.r2vTag,'String',num2str(t,2));

% extra load
hPoly = getappdata(0, 'hPoly');
fileName = getappdata(hPoly, 'ras2vecFileName');
ras2vec_output_fn = getappdata(hPoly, 'ras2vec_output_fn');
rows = getappdata(hPoly, 'rows');
cols = getappdata(hPoly, 'cols');

% call project.exe to invoke DP algorithm
param = sprintf('-e %f', t);
command = sprintf('ras2vec.exe  %s -p %s > temp', param, fileName);
system(command);

im = ones(rows,cols);
fig = getappdata(hPoly, getappdata(hPoly, 'ras2vec_figure'));
figure(fig);
show_epsilon_image(ras2vec_output_fn, im, 1, 'ras2vec', 0, fig);


% --- Executes on slider movement.
function mask_Callback(hObject, eventdata, handles)
% hObject    handle to mask (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'Value') returns position of slider
%        get(hObject,'Min') and get(hObject,'Max') to determine range of slider
t = get(handles.mask,'value');
set(handles.maskTag,'String',num2str(t,2));


function [lines] = get_lines_from_ras2vec(fn)
fid = fopen(fn,'r');
%fout = fopen('test.ply', 'w');
hPoly = getappdata(0, 'hPoly');
rows = getappdata(hPoly, 'rows');
lines = [];
i=0;
while 1
    tline = fgetl(fid);
    if ~ischar(tline), break, end
    if (strncmp(tline, 'POLY', 4))
       continue;
    end
	if (strncmp(tline, 'END', 3))
        i = 0;
		continue;
	end
	
    A = sscanf(tline, '%d', 2);
   
    if (i>0)
        lines = [x1 A(1)+1 y1 (rows-A(2)); lines];
%        fprintf(fout, '%d %d %d %d\n',[x1 A(1) y1 A(2)]);
    end

    x1 = A(1)+1;
    y1 = rows - A(2);

    i=i+1;
end
fclose(fid);
%fclose(fout);


function show_r2v_image(fn)

    fid = fopen(fn,'r');
    X=[]; Y=[];
    co = 'r';
    i=1;
    while 1
        tline = fgetl(fid);
        if ~ischar(tline), break, end

        A = sscanf(tline, '%d', 2);
        if A(1) == -1 & A(2) == -1
            i = 1;
            % 		line(X,Y,'Color', co);
            X=[]; Y=[];
            if (strcmp(co,'r'))
                co = 'b';
            else co = 'r';
            end
            continue;
        end

        X = [A(1), X];
        %Y = [392-A(2), Y];
        Y = [A(2), Y];

        if (i>1)
            X1 = [X(2) X(1)];
            Y1 = [Y(2) Y(1)];
            line(X1,Y1,'Color', co);
            if (strcmp(co,'r'))
                co = 'b';
            else co = 'r';
            end
        end

        i=i+1;
    end
    fclose(fid);


% --- Executes on button press in r2vBtn.
function r2vBtn_Callback(hObject, eventdata, handles)
% hObject    handle to r2vBtn (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

msg_handler = msgbox('Please waiting....', 'working....');

% preprocessed image
hPoly = getappdata(0, 'hPoly');
fn = getappdata(hPoly, 'preFileName');
all_lines = get_lines_from_ras2vec(getappdata(hPoly, 'ras2vec_output_fn'));
DIST_MASK = get(handles.mask,'value');  % default : 16; % distance mask

im_pre = imread(fn);
[rows cols notcare] = size(im_pre);
im = ones(rows,cols);
if notcare > 1,
    new_image = im2bw(im_pre);
else 
    new_image = im_pre;
end

hPoly = getappdata(0, 'hPoly');
HT_output_fn = getappdata(hPoly, 'HT_output_fn');

% line: x1, x2, y1, y2

% draw the new image (blank)
fig = getappdata(hPoly, 'DP_figure');
image_show(im, hPoly, 'DP algorithm with ras2vec output', 1, fig);

% compute the line length

%line([all_lines(i,1), all_lines(i,2)], [all_lines(i,3), all_lines(i,4)], 'Color', 'r');

all_points = get_points_for_DP(all_lines, DIST_MASK, new_image);

% write to file for DP algorithm.
write_to_file(HT_output_fn, all_points);

% show the results.
show_r2v_image(HT_output_fn);

% enable the epsilon slider
set(handles.epsilon, 'enable', 'on');

close(msg_handler);

function [dis] = manhanttan_dis(p1, p2)
dis = abs(p1(1)-p2(1)) + abs(p1(2)-p2(2));


% MOST IMPORTANT function for point reconstruction
% FUNCTION: get_points_for_DP
% INPUT:
%   all_lines:  the line set of the vectorized processing.
%   DIST_MASK:  the mask for distanct search
%   new_image:  the image containing data
% IMPORTANT THRESHOLD:
%   MIN_LEN_LINE:       the line threshold, can affect the line availability 
%   DIST_MASK:          the search mask
%   min(rows, cols)/4:  the search limitation. This could be factor for false line detection
% 
function [all_points] = get_points_for_DP(all_lines, DIST_MASK, new_image)
% compute the lines and pixels for the DP algorithm
[rows, cols, notcare] = size(new_image);

all_lines_len = (all_lines(:,1)-all_lines(:,2)).*(all_lines(:,1)-all_lines(:,2)) + (all_lines(:,3) - all_lines(:,4)).*(all_lines(:,3) - all_lines(:,4));
[len_sorted i] = max(all_lines_len);

MIN_LEN_LINE = 1; %8*8; % mininum length of the detected lines.
is_first_point_used = false;
first_point = [all_lines(i, 1) all_lines(i,3)];
cur_point   = [all_lines(i, 2) all_lines(i,4)]; % the pivot point
all_points = [cur_point; first_point]; % record the points aval.
cur_line = [first_point; cur_point]; % empty if it is image data, otherwise, the line on which cur_point lay.
processed_image = logical(ones(rows,cols));

my_disp(sprintf('starting point/lines: x1:%d y1:%d x2:%d y2:%d', first_point(1), first_point(2), cur_point(1), cur_point(2)));
[new_image, processed_image] = remove_data_around_line(new_image, cur_line, processed_image);
% figure; imshow(new_image);
% line([first_point(1) cur_point(1)],[first_point(2) cur_point(2)],'Color','r');

starting_point = first_point;
first_line = cur_line;
lines_included = [i];
line_num = 0;

while true
    
    % step 1. try to find all lines with one point in the distance less than DIST_MASK from cur_point
    [lines] = find_lines(cur_point, all_lines, DIST_MASK, lines_included, processed_image);
    
    % remove the lines close to the cur_points 
    lines_included = [lines lines_included];    
    %remove lines from all lines.
    
    [max_one i] = max(all_lines_len(lines));
    if length(lines) == 0 | max_one < MIN_LEN_LINE,  % if there no lines available, check image data as bridages.
        [image_data, new_image] = find_image_data(cur_point, DIST_MASK, new_image, processed_image);
%         disp('find_image_data');
%         size(new_image)
        if length(image_data) == 0,  % even no image data? reverse search or quit!
            if is_first_point_used == false,
                % reverse the all_points
                my_disp(sprintf('reverse point at x:%d y:%d after %d lines, cur_point is: x:%d y:%d', cur_point(1), cur_point(2), line_num, first_point(1), first_point(2)));
                is_first_point_used = true;

                % backup the end information
                temp_line = cur_line;
                temp_point = cur_point;

                cur_point = first_point;
                cur_line = first_line;
                
                % for next reverse (multiple reverse)
                first_line = temp_line;
                first_point = temp_point;
                
                starting_point = all_points(1,:);
                all_points = reverse_vector(all_points);
                line_num = 0;
                continue;
            else
                is_close = 0;
                if manhanttan_dis(cur_point, starting_point) < 2*DIST_MASK
                    all_points = [starting_point; all_points];
                    % add [-1 -1] as saparate for new polylines
                    is_closed = 1;
                    %all_points = [all_points; -1 -1];
                    break;
                else
                    % enlarge the search range.
                    DIST_MASK = DIST_MASK * 2;
                    
                    if (DIST_MASK > min(rows, cols)/4)  % 3 is a threshold
                        break;
                    end
                    
                    % re-start the search.
                    % is_first_point_used = false;
                    continue;
                end
%                disp(sprintf('break at x:%d y:%d after %d lines, cur lines:%d', cur_point(1), cur_point(2), line_num, length(lines)));
%                break;

                
                

                % recursive call
%                 temp_a = size(all_lines);
%                 temp_a = [1:temp_a(1)];
%                 new_lines = all_lines(find(ismember(temp_a, lines_included)==0), :);
%                 temp_a = size(new_lines)
%                 
%                 if (temp_a(1) > 15 & is_close == 1) % need improved
%                     all_points = [get_points_for_DP(new_lines, DIST_MASK, new_image); all_points];
%                     disp(sprintf('recursive at x:%d y:%d after %d lines, cur lines:%d', cur_point(1), cur_point(2), line_num, length(lines)));
%                 end
% 
%                 disp(sprintf('break at x:%d y:%d after %d lines, cur lines:%d', cur_point(1), cur_point(2), line_num, length(lines)));
%                 break;
            end
        else
            % update the all_points to including image data;
            % update cur_point, cur_line, all_points
%             if length(cur_line) > 0,
%                 size(new_image)
%                 figure; imshow(new_image);
%             end
            cur_line = [];
            image_data = int32(image_data);
            cur_point = [image_data(1,1) image_data(1,2)];
            all_points = [image_data; all_points];
            continue;
        end
    end
    % check the length of the lines.
    
    % check the lines valid
    i = lines(i);

    new_line = [all_lines(i, 1) all_lines(i,3) ; all_lines(i,2) all_lines(i, 4)];
    d1 = (cur_point(1) - new_line(1,1))^2 + (cur_point(2) - new_line(1,2))^2;
    d2 = (cur_point(1) - new_line(2,1))^2 + (cur_point(2) - new_line(2,2))^2;
    if length(cur_line) > 0, % previous there is a line
        %is_par = check_line_parallel(cur_line, new_line);
        %if (is_par == false)
        if 0
            inter_point = uint32(cal_intersection(cur_line, new_line));
            all_points(1, :) = inter_point;
            if d1 > d2
                cur_point = [new_line(1,1) new_line(1,2)];
            else
                cur_point = [new_line(2,1) new_line(2,2)];
            end
            all_points = [cur_point; all_points];
        else
            pre_point = cur_point;
            if d1 > d2
                cur_point = [new_line(1,1) new_line(1,2)];
                two_points = [cur_point; new_line(2,1) new_line(2,2)];
                close_point = [new_line(2,1) new_line(2,2)];
            else
                cur_point = [new_line(2,1) new_line(2,2)];
                two_points = [cur_point; new_line(1,1) new_line(1,2)];
                close_point = [new_line(1,1) new_line(1,2)];
            end
%             size(new_image)
            [mid_points, new_image] = get_middle_image_data(pre_point, close_point, new_image);
%         disp('mid');
%         size(new_image)
            all_points = [two_points; mid_points; all_points];
        end
    else % prevois is just a image data
        % update cur_point, cur_line, all_points
        pre_point = cur_point;
        if d1 > d2
            cur_point = [new_line(1,1) new_line(1,2)];
            two_points = [cur_point; new_line(2,1) new_line(2,2)];
            close_point = [new_line(2,1) new_line(2,2)];
        else
            cur_point = [new_line(2,1) new_line(2,2)];
            two_points = [cur_point; new_line(1,1) new_line(1,2)];
            close_point = [new_line(1,1) new_line(1,2)];
        end
        [mid_points, new_image] = get_middle_image_data(pre_point, close_point, new_image);
%         disp('mid2');
%         size(new_image)
        all_points = [two_points; mid_points; all_points];
    end

    %update cur_line;
    cur_line = new_line;
    [new_image, processed_image] = remove_data_around_line(new_image, new_line, processed_image);
    
    line_num = line_num + 1;
%     if (mod(fig_num, 2) == 0 & fig_num <0)
%     %if (fig_num < 10 & fig_num > 5)
%         figure; imshow(new_image);
%     end
    % 
end

% figure; imshow(new_image);
% figure; imshow(new_image);

function batch_pre_ras2vec_dp(hObject, eventdata, handles)
hPoly = getappdata(0, 'hPoly');
tempfn = getappdata(hPoly,'preFileName');
vecFile = getappdata(hPoly,'ras2vecFileName');
ras2vec_output_fn = getappdata(hPoly, 'ras2vec_output_fn');
HT_output_fn = getappdata(hPoly, 'HT_output_fn');

for k=0:78
    k
    if k < 10, str_pad = '00'; 
    elseif k < 100, str_pad = '0';
    else str_pad = '';
    end
    
    fileName=sprintf('data\\clear_slices\\slice_%s%d.tif',str_pad, k);
    output_fn = sprintf('tmp\\vec_slice_new_zfinal_%s%d.tif',str_pad, k);
    output_fn_pre = sprintf('tmp\\vec_slice_new_apre_%s%d.tif',str_pad, k);
    output_fn_vec = sprintf('tmp\\vec_slice_new_bvectized_%s%d.tif',str_pad, k);
    
    % preprocess the image to reduce noise
    
%     if get(handles.algTag, 'value') == 1
%         system(sprintf('src\\project.exe -P %s -o %s > temp', fileName, tempfn));
%     else 
%         system(sprintf('src\\project.exe -p %s -o %s > temp', fileName, tempfn));
%     end 

    system(sprintf('src\\project.exe -3 %s -o %s > temp', fileName, tempfn));
    
    preImg = imread(tempfn);
    ras2vecIm = im2bw(preImg);
    imwrite(ras2vecIm, vecFile, 'tif');
    imwrite(ras2vecIm, output_fn_pre, 'tif');
    
    % raster to vector conversion
    command = sprintf('ras2vec.exe  -p %s > temp', vecFile);
    system(command);
    
    % point chain generation
    all_lines = get_lines_from_ras2vec(ras2vec_output_fn);
    DIST_MASK = get(handles.mask,'value');  % default : 16; % distance mask

    new_image = ras2vecIm;
    [rows cols notcare] = size(new_image);
    im = ones(rows,cols);

    % show ras2vec image first
    fig = getappdata(hPoly, 'DP_figure');
    show_epsilon_image(ras2vec_output_fn, im, 1, 'ras2vec', 1, fig);    
    saveas(gcf, output_fn_vec, 'tif');
    close(getappdata(hPoly,'fig'));
    
    % get the ordered points, and write to a txt file, then load the coords
    % from the txt file and show the line segments.
    all_points = get_points_for_DP(all_lines, DIST_MASK, new_image);
    write_to_file(HT_output_fn, all_points);

    % show the line segments
    fig = getappdata(hPoly, 'DP_figure');
    image_show(im, hPoly, 'DP algorithm with ras2vec output', 1, fig);    
    show_r2v_image(HT_output_fn);
    
    saveas(gcf, output_fn, 'tif');
    close(getappdata(hPoly,'fig'));
end

function b_im = get_boundary(im)
fileName = 'get_boundary_tmp1.tif';
tempfn = 'get_boundary_tmp2.tif';
imwrite(im, fileName, 'tif');
system(sprintf('src\\project.exe -3 %s -o %s > temp', fileName, tempfn));
b_im = im2bw(imread(tempfn));
system(sprintf('del %s', fileName));
system(sprintf('del %s', tempfn));


function  im = my_im2bw(image)
im = image;
[rows, cols, notcare] = size(im);
if notcare > 1,
    im = im2bw(image);
end;

function ref_save_im = integrate_image(start_ref, end_ref)
ref_save_im=0;
for k = start_ref:end_ref
    if k < 10, str_pad = '00'; 
    elseif k < 100, str_pad = '0';
    else str_pad = '';
    end
    
    fileName=sprintf('data\\clear_slices\\slice_%s%d.tif',str_pad, k);
    tmp_im = my_im2bw(imread(fileName));
    if ref_save_im==0
        ref_save_im = tmp_im;
    else
        ref_save_im = ref_save_im & tmp_im;
    end
end
% Function: compute_landing
% 
% Return: image with colored pixels for showing the matching results
% Known issues:
function [result, ref, stat] = compute_landing(comp_res, cur_im, cur_im_boundary, ref_im, ref_im_boundary)
ref = ref_im;  % reference image to be updated.
cur_im_bound = find(cur_im_boundary == 0);
num_cur_im_bound = length(cur_im_bound);
ref_im_bound = find(ref_im_boundary == 0);
num_ref_im_bound = length(ref_im_bound);

[rows, cols]=size(cur_im);
result = uint8(255*ones(rows, cols, 3));

% GREEN - step 1, get the common landing points.
intersectB = intersect(cur_im_bound, ref_im_bound);
num_common = length(intersectB);
new_num_common = num_common;
indexB=intersectB;
for i = 1 : length(indexB)
    % x and y is calculated according to LP's algorithm.
    x = mod((indexB(i)-1), rows) + 1;
    y = floor((indexB(i)-1)/rows) + 1;
    result( x,y, :) = [0, 255, 0]';
end

% BLUE - step 2, get the missing data from new image.
% assumption: if there is no reference data in mask, then, mark it.
cur_im_boundary(intersectB) = 1;
cur_im_bound = find(cur_im_boundary == 0); % remaining pixels with common points removed.
indexB = cur_im_bound;
for i = 1 : length(indexB)
    % x and y is calculated according to LP's algorithm.
    x = mod((indexB(i)-1), rows) + 1;
    y = floor((indexB(i)-1)/rows) + 1;
    x1 = max(1, x-1); x2 = min(rows, x+1);
    y1 = max(1, y-1); y2 = min(cols, y+1);
    if length(find(ref_im_boundary(x1:x2, y1:y2) == 0)) == 0
        result(x, y, :) = [0, 0, 255]';
        ref(x,y) = 0;
    else %lady processing for step 3
        result(x, y, :) = [255, 0, 0]';
        
        % treat this as common pixels
        new_num_common=new_num_common+1; 
    end
end

% RED - step 3, get the noise data

stat1 = (new_num_common)/num_cur_im_bound*100;  % % of match on current new image
stat2 = (num_common)/num_ref_im_bound*100;      % % of match on reference image

fprintf(comp_res, sprintf('%10d', num_cur_im_bound));
fprintf(comp_res, sprintf('%10d', num_ref_im_bound));
fprintf(comp_res, sprintf('%10.2f%%', stat1));
fprintf(comp_res, sprintf('%10.2f%%', stat2));

stat = [stat1 stat2];
% Function:  batch_slices_comp
% this is batch function for slices comparison.
% the dxf model manully created is 
% <1-8>[8] => <9-15>[7] => [7] => [6] => [3] => [8] => [4] => [7] => [4] => [8]
% => [3|2 boundaries] => [9] => [5] => [12|decreasing] => [6] => [8] => [7]
%  total 112?
%
% Algorithm - 
% step 1: preprocess the image to get the boundary pixel  -3 means
    % sweep algorithm is used. keep the original image.
% step 2: compute the pixels with respect to the reference
    % image. Here, the integrated image only plays roles when the final
    % boundary are needed for this group.
% step 3: The parameters needed to be computed:
    % 1. the boundary pixels landing the same location,
    % 2. the boundary pixels in one side, but not in the other side. 
        % check whether it is long enough to form a feature.
        % maybe doing integrated is a good idea.
    % 3. the boundary pixels landing inside or outside.
    % 4. the trend of the boundary pixels.
function  batch_slices_comp(hObject, eventdata, handles)

hPoly = getappdata(0, 'hPoly');

comp_res = fopen('tmp/comp_result.txt', 'w');
fprintf(comp_res, 'image index | current | ref.  |comm/cur(%%)|comm/ref (%%)');
stat=[];
%for k=0:10
k=0;
NUM = 10;
start_ref = 0;
while true
    k
    if k < 10, str_pad = '00'; 
    elseif k < 100, str_pad = '0';
    else str_pad = '';
    end
    
    fprintf(comp_res, sprintf('\r\nslice_%s%d', str_pad, k));
    fileName=sprintf('data\\clear_slices\\slice_%s%d.tif',str_pad, k);
    resultName=sprintf('tmp/slice_result__%s%d.tif',str_pad, k);

    cur_im = my_im2bw(imread(fileName));

    if k == 0, 
        ref_im = cur_im;
        int_im = cur_im;
        k=k+1;
        continue; 
    end
    
    cur_im_boundary = get_boundary(cur_im); 
    ref_im_boundary = get_boundary(ref_im);
    int_im_boundary = get_boundary(int_im);
    
    [result_im, int_im, cur_stat] = compute_landing(comp_res, cur_im, cur_im_boundary, ref_im, ref_im_boundary);
    imwrite(result_im, resultName, 'tif');
    %fprintf(comp_res, sprintf('\r\nslice_%d', k));
    %result_im = compute_landing(comp_res, cur_im, cur_im_boundary, int_im, int_im_boundary);
    %fprintf(comp_res, '\n');

    stat = [cur_stat; stat];
    
    % update the images
    if (min(size(stat)) > 1 && stat(1,2) < 50 && stat(2,2) < 50)
        fprintf(comp_res, sprintf('\r\n\r\nA new reference image at slice %d\r\n', k-1) );
        ref_im = pre_im; % pre_im is the image of previous one. if we want to more backtracking, need more.
        ref_save_im = integrate_image(start_ref, k);
        k = k - 2;       % assume, k will be increased by 1.
        stat = [];
        start_ref = k;
        imwrite(ref_save_im, sprintf('tmp/accu_reference_%d.tif',k), 'tif');

        if k > NUM, break; end;
    end
    
    pre_im = cur_im;
    imwrite(int_im, sprintf('tmp/reference_%d.tif',k), 'tif');
    
    k=k+1;
    
end

fclose(comp_res);

% --- Executes on button press in batch.
function batch_Callback(hObject, eventdata, handles)
% hObject    handle to batch (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% this is batch processing funtion callback, we can substitute the disired
% functions by corresponding function calls.

% this is batch calls for HT line fitting on raw data only
%    batch_HT_raw(hObject, eventdata, handles);

% this is batch calls for pre-processing only
    batch_preprocess(hObject, eventdata, handles);


% this is batch calls for pre-processing, following by ras2vec, followed by
% ordered vertices search.
%    batch_pre_ras2vec_dp(hObject, eventdata, handles);

% this is batch calls for range data comparison.
%    batch_slices_comp(hObject, eventdata, handles);


function batch_HT_raw(hObject, eventdata, handles)
hPoly = getappdata(0, 'hPoly');

for k=0:110
    k
    if k < 10, str_pad = '00'; 
    elseif k < 100, str_pad = '0';
    else str_pad = '';
    end
    
    fileName=sprintf('data\\clear_slices\\slice_%s%d.tif',str_pad, k);
    setappdata(hPoly, 'curIm', imread(fileName));
    output_fn = sprintf('data\\pre_slices\\raw_HT_slice_%s%d.tif',str_pad, k);

    pushbutton5_Callback(hObject, eventdata, handles)
    saveas(gcf, output_fn, 'tif');
    close(getappdata(hPoly,'Hough transform'));    
end

function batch_preprocess(hObject, eventdata, handles)
hPoly = getappdata(0, 'hPoly');

for k=0:110
    k
    if k < 10, str_pad = '00'; 
    elseif k < 100, str_pad = '0';
    else str_pad = '';
    end
    
    fileName=sprintf('data\\clear_slices\\slice_%s%d.tif',str_pad, k);
    output_fn = sprintf('data\\pre_slices\\slice_%s%d.tif',str_pad, k);

    system(sprintf('src\\project.exe -3 %s -o %s > temp', fileName, output_fn));
end

function my_disp(str)
if 0
    disp(str);
end

function newIm = im_super_impose(curIm, initIm)
imSize = size(curIm);
if length(find((imSize==size(initIm)) == 0 )) > 0, 
    msg_handler = msgbox('the size of the new image is not consistent with the old one!', 'working....');
    return;
end;

newIm = curIm&initIm;


% --- Executes on button press in superimpose.
function superimpose_Callback(hObject, eventdata, handles)
% hObject    handle to superimpose (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

hPoly = getappdata(0, 'hPoly');
path = getappdata(hPoly, 'choose_path');
curIm = getappdata(hPoly, 'curIm');
if length(path)==0, path = ''; end;
[fileName, pathName] = uigetfile('*.tif; *.tiff; *.bmp', 'Choose a image file', path);

if fileName==0 return; end;

setappdata(hPoly, 'choose_path', pathName);

fileName = [pathName fileName];
initIm = imread(fileName);
[rows cols notcare] = size(initIm);
if (notcare > 1) initIm = im2bw(initIm); end;

initIm = im_super_impose(curIm, initIm);
setappdata(hPoly, 'curIm', initIm);
%image(initIm);
image_show(initIm, hPoly, fileName, 1, 'none');