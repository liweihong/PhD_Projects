function P = BgSub_background(mu,weight,sigma)

[rows, cols, K] = size(weight); % get the dimension.

imSize = rows*cols;
rgbSize = imSize * 3;
weight = reshape(weight, imSize, K); 
[Value, IndexK] = max(weight(:,:),[],2); % get the index of K into IndexK, size is imSize x 1

% Return the background based on the weight.
allIm = [1:imSize]';

P = ones(imSize, 3);
P(:,1) = mu ((IndexK - 1) * rgbSize + allIm);
P(:,2) = mu ((IndexK - 1) * rgbSize + imSize + allIm);
P(:,3) = mu ((IndexK - 1) * rgbSize + imSize*2 + allIm);
P = reshape(P, rows, cols, 3);
