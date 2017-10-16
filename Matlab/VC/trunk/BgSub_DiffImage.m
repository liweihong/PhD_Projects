function diffImUint8 = BgSub_DiffImage(firstIm,secondIm,thres)
% Get the difference of two images

[rows cols d] = size(firstIm);

% must use abs and double the uint8 image data
diffIm = abs(double(firstIm(:)) - double(secondIm(:)));
diffIm = reshape(diffIm, rows, cols, 3);
diffIndex = find(sum(diffIm,3)> thres);
diffImUint8 = ones(rows,cols);
diffImUint8(diffIndex) = 0;
	
end