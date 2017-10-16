% File: run_script_closed.m
% 
% This file is used to process the generated image file to get closed region after line fittings .
%
% Algorithm: Run the commands
%
function run_script_closed

data_dir = 'data/processed_post_noise_psu_inside';
out_dir = 'data/closed_processed_post_noise_psu_inside';
for i = 35:35
    % add the leading 0s.
    if i < 10, str_pad = '00'; 
    elseif i < 100, str_pad = '0';
    else str_pad = '';
    end
    
    filename=sprintf('%s/Done_Rotated_split_%s%d.tiff',data_dir, str_pad, i);
    image = uint8(imread(filename));
    % with data recover and preprocess 1
    % image = double(data_recover(preprocess(image)));
    % image = double(preprocess(image));  
    
    % with preprocess 2
    % image = double(preprocess_2(image));  
    im = fitting(image);
    
    out_filename = sprintf('%s/Closed_Rotated_split_%s%d.tiff',out_dir, str_pad, i);
    imwrite(im, out_filename, 'TIFF');
    disp(sprintf('%d finished!',i));
%    disp(filename);
%    disp(out_filename);
end

function im = process (image)

im = preprocess_2(image);


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


function newIm = dataRemove(pdetect, tetadetect, Imbinary);
% Function: drawLineOnData
% function [newIm, lineSeg] = drawLineOnData(pdetect, tetadetect, Imbinary);
% :
% --- Draw line based on Data, and remove the data which are close to lines.

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

    % Calculate the distance from a point to a line
    % Equation 11
    diffX = x2 - x1;
    diffY = y2 - y1;
    disPow = diffX^2 + diffY^2;
    deSq = sqrt(disPow);
    dArray = abs(diffX*(y1-DataY) - diffY*(x1-DataX))/deSq; % Distance equation from "http://mathworld.wolfram.com/Point-LineDistance2-Dimensional.html"
    lambdaThresh = 4; % another important threshold - DISTANCE which is CLOSE enough to be removed.
    dArray = find(dArray < lambdaThresh);
    newIm(DataY(dArray), DataX(dArray)) = 1; % Remove the data fitted into a line.
end  % end of for

function lineSeg = lineSegCompute(pdetect, tetadetect, Imbinary);
% Function: drawLineOnData
% function [newIm, lineSeg] = drawLineOnData(pdetect, tetadetect, Imbinary);
% :
% --- Draw line based on Data, and remove the data which are close to lines.

lineSeg = [];
[rows cols] = size(Imbinary);
newIm = Imbinary;
for i = 1:length(pdetect)
    [DataY DataX] = find(newIm == 0); % Matlab return value is x <-> y;
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

    % Calculate the distance from a point to a line
    % Equation 11
    diffX = x2 - x1;
    diffY = y2 - y1;
    disPow = diffX^2 + diffY^2;
    deSq = sqrt(disPow);
    dArray = abs(diffX*(y1-DataY) - diffY*(x1-DataX))/deSq; % Distance equation from "http://mathworld.wolfram.com/Point-LineDistance2-Dimensional.html"
    lambdaThresh = 4; % another important threshold - DISTANCE which is CLOSE enough to be removed.
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
                % sort the coordinates for closed polygon
                if ptAllx(indexStart) > ptAllx(accum)
                    lineSeg = [lineSeg;ptAllX(indexStart) ptAllX(accum) ptAllY(indexStart) ptAllY(accum)];
                else 
                    lineSeg = [lineSeg;ptAllX(accum) ptAllX(indexStart) ptAllY(accum) ptAllY(indexStart)];
                end
            end
            accum = accum + 1;
        end
        
    end
    
end


% function my_line: to draw the line on image im, based on lineSeg
% coordinates
function im = my_line(im, lineSeg)

[len temp] = size(lineSeg);
for i = 1:len
    x0 = uint32(lineSeg(i,1)); y0 = uint32(lineSeg(i,3));
    x1 = uint32(lineSeg(i,2)); y1 = uint32(lineSeg(i,4)); 
    % see which axis is longer, then, walk in that axis direction.
    absX = abs(x1-x0);
    absY = abs(y1-y0);
    if absX == 0 % Vertical Line
    	for ii = y0:y1, 
    		im(ii, x0, 1) = 255;
    		im(ii, x0, 2) = 0;
    		im(ii, x0, 3) = 0;
    	end
    	continue;
    end

    if absY == 0 % Horizantol Line
    	for ii = x0:x1, 
    		im(y0, ii, 1) = 255;
    		im(y0, ii, 2) = 0;
    		im(y0, ii, 3) = 0;
    	end
    	continue;
    end
    
    m_tg = absY/absX;

    if absX > absY
    	for j = 1:absX % draw the lines
    		tempX = uint32(x0 + (j-1));
    		tempY = uint32(y0 + m_tg*(j-1));
    		im(tempY, tempX, 1) = 255;
    		im(tempY, tempX, 2) = 0;
    		im(tempY, tempX, 3) = 0;
    	end
    else 
    	for j = 1:absY % draw the lines
    		tempX = uint32(x0 + (j-1)/m_tg);
    		tempY = uint32(y0 + (j-1));
    		im(tempY, tempX, 1) = 255;
    		im(tempY, tempX, 2) = 0;
    		im(tempY, tempX, 3) = 0;
    	end
    end
    
end

% function my_line: to draw the line on image im, based on lineSeg
% coordinates
function im = closed_poly(im, lineSeg)

[rows cols] = size(im);

% sort the line segments pairs
[t index] = sort(lineSeg(:,1));


% use a scanning algorithm to get closed polygon


% function fitting: fit the image
function im = fitting(Imbinary)
thresh = 40; % 16; set the initial threshold for HT.
factor = 1.5; % 2; set the division factor.
% thresh = get(handles.initThread,'value');
% factor = get(handles.IDFTag, 'value');
%stopThresh = 4.0; % set the stopping threshold for HT.

[rows cols] = size(Imbinary);
im = uint8(255*ones(rows,cols,3));
% [Y X] = find(Imbinary == 0);
% len=length(X);
% for i=1:len, 
% 	im(Y(i), X(i), :) = 0; 
% end;

pd = []; td = [];
oldIm = Imbinary;
% Recursive HT.
while true
    % Hough Transform to get the lines.
    % Strategy Pattern for Algorithms for C++
    [pdetect,tetadetect,Accumulator] = houghTransform(Imbinary,thresh);

    pd = [pd; pdetect];
    td = [td; tetadetect];
    % Remove the fitted data
    Imbinary = dataRemove(pdetect, tetadetect, Imbinary);
    
    if factor == 1 break; end;
    % If all data points are fitted, just break.
    len = length(find(Imbinary==0));
    if len == 0
        break;
    end

    % adjust the threshold to smaller one.
    thresh = thresh / factor;
end

    lineSeg = lineSegCompute(pd, td, oldIm);
    % Draw Line Segmentation now
    % im = my_line(im, lineSeg);
    im = closed_poly(im, lineSeg);
