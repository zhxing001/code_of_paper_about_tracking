function k = dense_gauss_kernel(sigma, x, y)
%DENSE_GAUSS_KERNEL Gaussian Kernel with dense sampling.
%   Evaluates a gaussian kernel with bandwidth SIGMA for all displacements
%   between input images X and Y, which must both be MxN. They must also
%   be periodic (ie., pre-processed with a cosine window). The result is
%   an MxN map of responses.
%   %result 是一个响应图
%  
%
%   Jo鉶 F. Henriques, 2012
%   http://www.isr.uc.pt/~henriques/
	
	xf = fft2(x);         %x in Fourier domain
	xx = x(:)' * x(:);    %squared norm of x  二范数 为生成高斯核用，径向基核函数
		
	if nargin >= 3,     %general case, x and y are different   这个量统计得是函数输入得参数个数
		yf = fft2(y);
		yy = y(:)' * y(:);
	else
		%auto-correlation of x, avoid repeating a few operations
		yf = xf;
		yy = xx;
	end

	%cross-correlation term in Fourier domain   互相关项（傅里叶域中）
	xyf = xf .* conj(yf);   %对应相乘,yf取复共轭,这就是对应相乘，是RBF得相关项
	xy = real(circshift(ifft2(xyf), floor(size(x)/2)));  %to spatial domain  变换到空间域
   
	
	%calculate gaussian response for all positions
	k = exp(-1 / sigma^2 * max(0, (xx + yy - 2 * xy) / numel(x)));
    %numel(x) 是指x中元素得多少

end

