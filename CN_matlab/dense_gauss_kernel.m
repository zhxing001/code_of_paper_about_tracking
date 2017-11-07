function k = dense_gauss_kernel(sigma, x, y)

% k = dense_gauss_kernel(sigma, x, y)
%
% Computes the kernel output for multi-dimensional feature maps x and y
% using a Gaussian kernel with standard deviation sigma.

xf = fft2(x);      %x in Fourier domain
xx = x(:)' * x(:);  %squared norm of x

if nargin >= 3,  %general case, x and y are different
    yf = fft2(y);
    yy = y(:)' * y(:);
else
    %auto-correlation of x, avoid repeating a few operations
    yf = xf;
    yy = xx;
end

%cross-correlation term in Fourier domain
xyf = xf .* conj(yf);          %对应维相乘
xy = real(ifft2(sum(xyf,3)))/3;    %to spatial domain,这里是复数域直接加起来，然后在整体求傅里叶变换，和KCF有点区别
%KCF是每个通道作ifft之后然后加起来，而且这里我觉得加起来除以三比较好，求一个均值合理一些，毕竟是把三个通道的值加起来了啊。
%我用除了三之后代码也做了实验，对结果几乎没有影响，准确率和帧率都没有什么影响

%calculate gaussian response for all positions
k = exp(-1 / sigma^2 * max(0, (xx + yy - 2 * xy) / numel(x)));
end