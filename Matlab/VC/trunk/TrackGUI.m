function varargout = TrackGUI(varargin)
% TrackGUI M-file for TrackGUI.fig
%      TrackGUI, by itself, creates a new TrackGUI or raises the existing
%      singleton*.
%
%      H = TrackGUI returns the handle to a new TrackGUI or the handle to
%      the existing singleton*.
%
%      TrackGUI('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in TrackGUI.M with the given input arguments.
%
%      TrackGUI('Property','Value',...) creates a new TrackGUI or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before TrackGUI_OpeningFunction gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to TrackGUI_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help TrackGUI

% Last Modified by GUIDE v2.5 14-May-2006 13:22:37

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @TrackGUI_OpeningFcn, ...
                   'gui_OutputFcn',  @TrackGUI_OutputFcn, ...
                   'gui_LayoutFcn',  [] , ...
                   'gui_Callback',   []);
if nargin & isstr(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end
% End initialization code - DO NOT EDIT


% --- Executes just before TrackGUI is made visible.
function TrackGUI_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to TrackGUI (see VARARGIN)

% Choose default command line output for TrackGUI
handles.output = hObject;

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes TrackGUI wait for user response (see UIRESUME)
% uiwait(handles.figure1);
fileName = get(handles.inEdit,'String');
if ~isempty(fileName)
    mov = aviread(fileName, 1);
    initIm = circshift(mov(1).cdata, [0,0, -1]);
    hOrgIm = getappdata(0, 'hOrgIm');
    hNewIm = getappdata(0, 'hNewIm');
    axes(hOrgIm);
    imshow(initIm);
    setappdata(hOrgIm,'fileName',fileName);
    setappdata(hOrgIm,'img',initIm);
    axes(hNewIm);
    imshow(initIm);
end;

% --- Outputs from this function are returned to the command line.
function varargout = TrackGUI_OutputFcn(hObject, eventdata, handles)
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;


% --- Executes during object creation, after setting all properties.
function edit3_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit3 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end



% --- Executes during object creation, after setting all properties.
function inEdit_CreateFcn(hObject, eventdata, handles)
% hObject    handle to inEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end



function inEdit_Callback(hObject, eventdata, handles)
% hObject    handle to inEdit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of inEdit as text
%        str2double(get(hObject,'String')) returns contents of inEdit as a double


% --- Executes on button press in loadPush.
function loadPush_Callback(hObject, eventdata, handles)
% hObject    handle to loadPush (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

fileName = uigetfile('*.avi', 'Choose a video file');

if ~isempty(fileName)
    set(handles.inEdit, 'String', fileName);
    mov = aviread(fileName, 1);
    initIm = circshift(mov(1).cdata, [0,0, -1]);
    hOrgIm = getappdata(0, 'hOrgIm');
    hNewIm = getappdata(0, 'hNewIm');
    setappdata(hOrgIm,'fileName',fileName);
    setappdata(hOrgIm,'img',initIm);
    axes(hOrgIm);
    imshow(initIm);
    axes(hNewIm);
    imshow(initIm);
end;

% --- Executes on button press in transfCheck.
function transfCheck_Callback(hObject, eventdata, handles)
% hObject    handle to transfCheck (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of transfCheck


% --- Executes during object creation, after setting all properties.
function intSlider_CreateFcn(hObject, eventdata, handles)
% hObject    handle to intSlider (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: slider controls usually have a light gray background, change
%       'usewhitebg' to 0 to use default.  See ISPC and COMPUTER.
usewhitebg = 1;
if usewhitebg
    set(hObject,'BackgroundColor',[.9 .9 .9]);
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end


% --- Executes on slider movement.
function intSlider_Callback(hObject, eventdata, handles)
% hObject    handle to intSlider (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'Value') returns position of slider
%        get(hObject,'Min') and get(hObject,'Max') to determine range of slider
t = get(handles.intSlider,'value');
set(handles.valText,'String',num2str(t,2));
hOrgIm = getappdata(0, 'hOrgIm');
setappdata(hOrgIm, 'alpha',t)

% --- Executes on button press in appPush.
function appPush_Callback(hObject, eventdata, handles)
% hObject    handle to appPush (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --- Executes on button press in closePush.
function closePush_Callback(hObject, eventdata, handles)
% hObject    handle to closePush (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
close all;





% --- Executes during object creation, after setting all properties.
function orgIm_CreateFcn(hObject, eventdata, handles)
% hObject    handle to orgIm (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: place code in OpeningFcn to populate orgIm

setappdata(0, 'hOrgIm', hObject);



% --- Executes during object creation, after setting all properties.
function newIm_CreateFcn(hObject, eventdata, handles)
% hObject    handle to newIm (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: place code in OpeningFcn to populate newIm

setappdata(0, 'hNewIm', hObject);


% --- Executes on button press in pushbutton13.
function pushbutton13_Callback(hObject, eventdata, handles)
% hObject    handle to pushbutton13 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
hOrgIm = getappdata(0, 'hOrgIm');
initIm = getappdata(hOrgIm, 'img');
axes(hOrgIm);
[roi x y]=roipoly(initIm);
roi = find(roi);  %%%%%%%%% ALWASY use find after roipoly();
setappdata(hOrgIm, 'roi', roi);
[rows cols d] = size(initIm);
initIm = ones(rows, cols);
%initIm(roi) = 0;
%imshow(initIm);
line(x, y, 'LineWidth', 2, 'Color', 'r');
set(handles.pushbutton14, 'Enable', 'on');
set(handles.pushbutton15, 'Enable', 'on');

% --- Executes on button press in pushbutton14.
function pushbutton14_Callback(hObject, eventdata, handles)
% hObject    handle to pushbutton14 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

hOrgIm = getappdata(0, 'hOrgIm');
hNewIm = getappdata(0, 'hNewIm');
fileName = getappdata(hOrgIm,'fileName');
roi = getappdata(hOrgIm, 'roi');
setappdata(hOrgIm, 'stop', 0);
BgSub_ROI(roi, 1, fileName, hOrgIm, hNewIm);
disp('Stop tracking');


% --- Executes on button press in pushbutton15.
function pushbutton15_Callback(hObject, eventdata, handles)
% hObject    handle to pushbutton15 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
hOrgIm = getappdata(0, 'hOrgIm');
setappdata(hOrgIm, 'stop', 1);
