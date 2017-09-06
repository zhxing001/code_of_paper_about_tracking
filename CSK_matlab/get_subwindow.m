function out = get_subwindow(im, pos, sz, cos_window)
% GET_SUBWINDOW Obtain sub-window from image, with replication-padding.
%   Returns sub-window of image IM centered at POS ([y, x] coordinates),
%   with size SZ ([height, width]). If any pixels are outside of the image,
%   they will replicate the values at the borders.
%
%   The subwindow is also normalized to range -0.5 .. 0.5, and the given
%   cosine window COS_WINDOW is applied (though this part could be omitted
%   to make the function more general).
%
%   Jo鉶 F. Henriques, 2012
%   http://www.isr.uc.pt/~henriques/

	if isscalar(sz),     %square sub-window  确定输入是否为标量,如果只输入一个参数，长宽都是如此的话就是标量，这里不是
		sz = [sz, sz];   %方形子窗口,
	end
	
	xs = floor(pos(2)) + (1:sz(2)) - floor(sz(2)/2);
	ys = floor(pos(1)) + (1:sz(1)) - floor(sz(1)/2);
	%这里是获得一个矩形框，就是根据所给的POS和sz信息获得这个矩形框的行列索引，
	%check for out-of-bounds coordinates, and set them to the values at
	%处理边界情况
    %the borders
	xs(xs < 1) = 1;
	ys(ys < 1) = 1;
	xs(xs > size(im,2)) = size(im,2);
	ys(ys > size(im,1)) = size(im,1);
	
   
	%extract image
	out = im(ys, xs, :);
	
	%pre-process window
	out = double(out) / 255 - 0.5;  %normalize to range -0.5 .. 0.5   归一化到-0.5--0.5
	out = cos_window .* out;      %apply cosine window  %对应位置相乘  加上余弦窗,突出中心位置

end

