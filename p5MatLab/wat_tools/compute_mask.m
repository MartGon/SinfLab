function mask = compute_mask(Image,kernelsize)
%compute_mask Returns the perceptual mask of Image using a 
%     Gaussian edge detector with std=kernelsize 
%     The mask has the same size as the input image
[imx,imy]=gaussgradient(Image,kernelsize);
mask = sqrt(imx.^2 + imy.^2);
end

