%### Background subtraction for motion.

% Locate the background frame manually. (around 38 secs position, 36 sec is good.)
% bgFrame = aviread('motion.avi', 30*36);
% figure;imshow(circshift(bgFrame(1).cdata, [0,0, -1]));

%### Define some parameters =====

IMT = 5;  % corresponding to t, the number of frames in the history.
ALPHA = 0.02; % corresponding to alpha, learning rate.
K = 3; % correspondig to K, the number of mixtures of Gaussian.
T = 0.1; % corresponding to T, the threshold for background.

%### Read frames from video clip.
aviInfo = aviinfo('background.avi');
frameNum = aviInfo.NumFrames;
bgMov = aviread('background.avi', 1);
bgFrame = circshift(bgMov(1).cdata, [0,0, -1]);
[rows cols d] = size(bgFrame);


%### Some global variables
weight = ones(rows, cols, K);
mu = uint8(ones(rows, cols, d, K)); % must be uint8 to display the image.
sigma = ones(rows, cols, K);
weight(:,:,1) = 1/K; weight(:,:,2) = 1/K; weight(:,:,3) = 1/K; % Initial to equal weight.
mu(:,:,:,1) = bgFrame; mu(:,:,:,2) = 0; mu(:,:,:,3) = 0;
sigma(:,:,1) = 25; sigma(:,:,2) = 100; sigma(:,:,3) = 100; % sigma is the variance, instead of SD.

%### Gaussian Mixture model for background modeling.
for i = 4 : 3 : frameNum,

	bgMov = aviread('background.avi', i);
	bgFrame = circshift(bgMov(1).cdata, [0,0, -1]);
	
	%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	% Update the background model using GMM
	
	% Match up test, suppose only up to one Gaussian distribution will be matched up.
	% Because of the operation flow (didn't find, then, remove one distribution).
	
	matchIndexAll = [];
	for j = 1 : K
        %### Get the match score
		[matchScore  allVar]= BgSub_GassMatch(bgFrame, mu(:,:,:,j), sigma(:,:,j));
		matchScore = reshape(matchScore, rows, cols);
		matchMatrix = matchScore<3; % 3*sigma(:,:,1) is not correct
		matchIndex = find(matchMatrix); % 3 sigma
		matchIndexAll = union(matchIndex, matchIndexAll);
		%matchIm = zeros(rows,cols);
		%matchIm(matchIndex) = 1;
		%figure; imshow(matchIm);

		%### Update the mixture model
		weight(:,:,j) = weight(:,:,j) + ALPHA * (matchMatrix - weight(:,:,j)); % update the weight.
		rho = ALPHA * ((1./((2*pi)^1.5*(sigma(:,:,j).^0.5))).*exp(matchScore)); % get the RHO to update mean and variance.
		tempMu = BgSub_ReshapeRGB(mu(:,:,:,j));   % Reshape RGB to Nx3 matrix. can use reshape directly;
		bgFrameReshape = BgSub_ReshapeRGB(bgFrame);
		
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
	imSize = rows*cols;
    rgbSize = imSize*3;
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
	if mod(i, 30) == 1
		%### Background Model Estimation
		%### Update mu to bgFrame for saving
		fileName = sprintf('images/frame%d.jpg',i);
		bgFrame = uint8(BgSub_background(mu, weight, sigma));
		imwrite(bgFrame, fileName, 'jpg');
		disp(fileName);
	end
	
	disp(i);
end	 % end of for of the entire video


