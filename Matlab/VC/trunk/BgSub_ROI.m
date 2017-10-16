function P = BgSub_ROI(roi, numDisp, fileName, hOrgIm, hNewIm) 

%### Define some parameters =====
ALPHA = 0.1; % corresponding to alpha, learning rate.
K = 3; % correspondig to K, the number of mixtures of Gaussian.
tempVal = getappdata(hOrgIm, 'alpha');
if tempVal ~= []  ALPHA = tempVal; end

%### Read frames from video clip.
aviInfo = aviinfo(fileName);
frameNum = aviInfo.NumFrames;
initFrame = getappdata(hOrgIm, 'img');
[rows cols d] = size(initFrame);
imSize = rows*cols;
rgbSize = imSize*3;

%### Some global variables
weight = ones(rows, cols, K);
mu = uint8(ones(rows, cols, d, K)); % must be uint8 to display the image.
sigma = ones(rows, cols, K);
%roiInit = 0.98*ones(rows, cols);
%roiInit(roi) = 0.02;
%weight(:,:,1) = roiInit; weight(:,:,2) = 1 - roiInit; weight(:,:,3) = 1 - roiInit; % Initial to equal weight.
weight(:,:,1) = 1/K; weight(:,:,2) = 1/K; weight(:,:,3) = 1/K; % Initial to equal weight.

mu(:,:,:,1) = initFrame;  mu(:,:,:,2) = 0; mu(:,:,:,3) = 0;
sigma(:,:,1) = 25; sigma(:,:,2) = 100; sigma(:,:,3) = 100; % sigma is the variance, instead of SD.

%### Gaussian Mixture model for background modeling.
startPos = 4;
stepLen = 3;
for i = startPos : stepLen : frameNum,

    askStop = getappdata(hOrgIm, 'stop');
    if (askStop == 1) return; end;
        
	bgMov = aviread(fileName, i);
	bgFrame = circshift(bgMov(1).cdata, [0,0, -1]);
	
	%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	% Update the background model using GMM
	
	% Match up test, suppose only up to one Gaussian distribution will be matched up.
	% Because of the operation flow (didn't find, then, remove one distribution).
	
	matchIndexAll = [];

    for j = 1 : K
        %### Get the match score
		[matchScore  allVar]= GassMatch(bgFrame, mu(:,:,:,j), sigma(:,:,j));

        matchScore = reshape(matchScore, rows, cols);
		matchMatrix = matchScore<2; % 3*sigma(:,:,1) is not correct
		matchIndex = find(matchMatrix); % 3 sigma
		matchIndexAll = union(matchIndex, matchIndexAll);
		%matchIm = zeros(rows,cols);
		%matchIm(matchIndex) = 1;
		%figure; imshow(matchIm);

		%### Update the mixture model
		weight(:,:,j) = weight(:,:,j) + ALPHA * (matchMatrix - weight(:,:,j)); % update the weight.
		rho = ALPHA * ((1./((2*pi)^1.5*(sigma(:,:,j).^0.5))).*exp(matchScore)); % get the RHO to update mean and variance.
		tempMu = double(reshape(mu(:,:,:,j), imSize, 3));   % Reshape RGB to Nx3 matrix. can use reshape directly;
		bgFrameReshape = double(reshape(bgFrame, imSize, 3));
		
		tempMu(matchIndex,1) = (1 - rho(matchIndex)).*tempMu(matchIndex,1) + rho(matchIndex).*bgFrameReshape(matchIndex,1);
		tempMu(matchIndex,2) = (1 - rho(matchIndex)).*tempMu(matchIndex,2) + rho(matchIndex).*bgFrameReshape(matchIndex,2);
		tempMu(matchIndex,3) = (1 - rho(matchIndex)).*tempMu(matchIndex,3) + rho(matchIndex).*bgFrameReshape(matchIndex,3);
		tempMu = reshape(tempMu, rows, cols, 3); % reshape back to RGB image;
		mu(:,:,:,j) = tempMu;
		
		tempSig = sigma(:,:,j);  % update the variance.
		tempSig(matchIndex) = (1 - rho(matchIndex)).*tempSig(matchIndex) + rho(matchIndex).*allVar(matchIndex);
		tempSig = reshape(tempSig, rows, cols);
		sigma(:,:,j) = tempSig;
    end % end of for of K GMM.
	
	%### No match found;
	noMatchIndex = [1:imSize]; % row vector;
	noMatchIndex = setdiff(noMatchIndex', matchIndexAll);  % column vector

	%matchIm = ones(rows,cols);
	%matchIm(noMatchIndex) = 0;
	%figure; imshow(matchIm);
    
    weightTemp = reshape(weight, imSize, K);
	[Y, IndexK] = min(weightTemp(noMatchIndex,:),[],2); % get the index of K into IndexK
	weight((IndexK-1)*imSize + noMatchIndex) = 0.01; % set the weight to a low prior weight.
	sigma((IndexK-1)*imSize + noMatchIndex) = 100; % set the initially high variance.
	mu((IndexK-1)*rgbSize + noMatchIndex) = bgFrameReshape(noMatchIndex,1);  % set RGB
	mu((IndexK-1)*rgbSize + noMatchIndex + imSize) = bgFrameReshape(noMatchIndex,2); % set RGB
	mu((IndexK-1)*rgbSize + noMatchIndex + imSize*2) = bgFrameReshape(noMatchIndex,3); % set RGB

    % return;
	% difference image
	% diffImUint8 = BgSub_DiffImage(bgFrame, mu(:,:,:,1), 40);
	% figure; imshow(diffImUint8);

	%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

	%### Save the background image.
	if mod(i - startPos, numDisp*stepLen) == 0
		%### Background Model Estimation
		%### Update mu to bgFrame for saving
        axes(hOrgIm);
        imshow(bgFrame);
		bgFrame = uint8(background(roi, initFrame, mu, weight, sigma));
        axes(hNewIm);
        imshow(bgFrame);
	end
	disp(i);
end	 % end of for of the entire video

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function [P, Q] = GassMatch(im,mu,sigma)

%all_R = double(im(:,:,1)); all_G = double(im(:,:,2)); all_B = double(im(:,:,3)); 
%all_data = [all_R(:),all_G(:),all_B(:)]; % that's the entire image data 
%all_R = double(mu(:,:,1)); all_G = double(mu(:,:,2)); all_B = double(mu(:,:,3)); 
%all_mu = [all_R(:),all_G(:),all_B(:)]; % that's the mean image
all_data = reshape(im, 320*240, 3);
all_mu = reshape(mu, 320*240, 3);
all_sigma = sigma(:); 			% vector

all_data = all_data - all_mu; % difference matrix
P = all_data.*all_data; 
P = sum(P,2); 					% squred and summation
Q = P;
P = P.*(1./(all_sigma));  % get the final equation.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function P = background(roi, initFrame, mu,weight,sigma)

[rows, cols, K] = size(weight); % get the dimension.

imSize = rows*cols;
rgbSize = imSize * 3;
weight = reshape(weight, imSize, K); 
[Value, IndexK] = max(weight(roi,:),[],2); % get the index of K into IndexK, size is imSize x 1

P = initFrame;  % Initial in case of ...
%P = mu(:,:,:,1);
P = reshape(P, imSize, 3);
P(roi,1) = mu ((IndexK - 1) * rgbSize + roi);
P(roi,2) = mu ((IndexK - 1) * rgbSize + imSize + roi);
P(roi,3) = mu ((IndexK - 1) * rgbSize + imSize*2 + roi);
P = reshape(P, rows, cols, 3);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
