function [f d] = siftreg(I, spacing, patchwidth, orient);
% spacing and patchwidth are both in pixels

if nargin < 4
    % By default, do not compute orientation, assume to be 0
    orient = 0;
end

I = im2double(I);
if size(I,3) > 1
    I = rgb2gray(I) ;
end
M = size(I,1);
N = size(I,2);

sigma = patchwidth/2/6; %patch radius will be 6*sigma

xpts = patchwidth/2:spacing:N-patchwidth/2;
ypts = patchwidth/2:spacing:M-patchwidth/2;
[X Y] = meshgrid(xpts, ypts);
npts = length(X(:));
f = [X(:) Y(:) sigma*ones(npts,1) zeros(npts,1)];

f = f';

% SIFT features
if orient
    [f,d] =  sift(single(I), 'frames', f, 'orientations') ;
else
    [f,d] =  sift(single(I), 'frames', f) ;
end