function [img_files, pos, target_sz, resize_image, ground_truth, ...
	video_path] = load_video_info(video_path)
%LOAD_VIDEO_INFO
%   Loads all the relevant information for the video in the given path:
%   the list of image files (cell array of strings), initial position
%   (1x2), target size (1x2), whether to resize the video to half
%   (boolean), and the ground truth information for precision calculations
%   (Nx2, for N frames). The ordering of coordinates is always [y, x].
%
%   The path to the video is returned, since it may change if the images
%   are located in a sub-folder (as is the default for MILTrack's videos).
%   翻译如下：
%   返回给定路径中视频的所有相关信息：
%   1：文件列表。
%   2：初始位置（1*2）
%   3：目标大小（1*2）
%   4：是否缩放图片（bool型）
%   5：ground_truth（N*2）,一共N帧，坐标先y后x.
%      这里面存的是目标再每一帧的准确位置，为统计准确率用。
%   图像路径也返回，因为如果存在子文件夹，这个路径也会发生变化    
%   Jo鉶 F. Henriques, 2012
%   http://www.isr.uc.pt/~henriques/

	%load ground truth from text file (MILTrack's format)
	text_files = dir([video_path '*_gt.txt']);
    %获得结尾为_gt.txt的文件。
	assert(~isempty(text_files), 'No initial position and ground truth (*_gt.txt) to load.')
    %这个函数的结果是抛出异常，语法参考函数帮助,如果gt.txt没有参数就抛出异常
	f = fopen([video_path text_files(1).name]);
    %打开gt文件
	ground_truth = textscan(f, '%f,%f,%f,%f');    %[x, y, width, height]
    %获得所有的数
	ground_truth = cat(2, ground_truth{:});
    %沿制定维度链接数组，2表示横向，具体参考函数帮助
	fclose(f);
	
	%set initial position and size
	target_sz = [ground_truth(1,4), ground_truth(1,3)];
    %初始大小，由gt.txt的第一行来获得
	pos = [ground_truth(1,2), ground_truth(1,1)] + floor(target_sz/2);
	%初始位置：由gt.txt的第一行来获得，坐标和行列恰好相反。这里加上大小的一半？
    %gt.txt里给的位置是左上角的位置？   
    
	%interpolate missing annotations, and store positions instead of boxes
    %gt.txt里给的位置是每五帧给一个，所以要进行插值确定缺失的位置
	try
		ground_truth = interp1(1 : 5 : size(ground_truth,1), ...
			ground_truth(1:5:end,:), 1:size(ground_truth,1));
        %线性插值，看函数帮助。
		ground_truth = ground_truth(:,[2,1]) + ground_truth(:,[4,3]) / 2;
        %获得所有的位置
	catch  %#ok, wrong format or we just don't have ground truth data.
		ground_truth = [];
    end
    %到这里就得到所有帧目标的中心位置，这里主要是为了评价跟踪效果用的。
	
    
	%list all frames. first, try MILTrack's format, where the initial and
	%final frame numbers are stored in a text file. if it doesn't work,
	%try to load all png/jpg files in the folder.
	%获得所有帧，首先，先去看下有没有format文件，可以获得帧数信息，如果没有的话就去找所有的png或者jpg文件
    
	text_files = dir([video_path '*_frames.txt']);
	if ~isempty(text_files),
		f = fopen([video_path text_files(1).name]);
		frames = textscan(f, '%f,%f'); 
        %打开文件获得信息。
		fclose(f);
		
		%see if they are in the 'imgs' subfolder or not
		if exist([video_path num2str(frames{1}, 'imgs/img%05i.png')], 'file'),
            %img%05i.png的意思是看文件夹里是否有编号为 0001，0002且后缀为jpg或png的图片，如果有返回TRUE
			video_path = [video_path 'imgs/'];   %如果存在图片，接更新video_path
        elseif ~exist([video_path num2str(frames{1}, 'img%05i.png')], 'file'),
			error('No image files to load.')
		end
		
		%list the files
		img_files = num2str((frames{1} : frames{2})', 'img%05i.png');
        %这个函数就是把数字转换为字符串,没有用dir函数可能是避免排序吧，下面的else就是用dir做的，这样获得的字符串就可以直接用
		img_files = cellstr(img_files);
        %吧字符串变为字符串细胞组
	else
		%no text file, just list all images
		img_files = dir([video_path '*.png']);
		if isempty(img_files),
			img_files = dir([video_path '*.jpg']);
			assert(~isempty(img_files), 'No image files to load.')
		end
		img_files = sort({img_files.name});
	end
	
	
	%if the target is too large, use a lower resolution - no need for so
	%much detail
    %如果目标太大（这回造成跟踪时计算量增加，可以resize），如果小于100
	if sqrt(prod(target_sz)) >= 100,   %如果长宽比大于100就缩小
        %prod是把矩阵的每一列连乘起来。如果只有一行就把这一行连乘，其他的用法看函数帮助
		pos = floor(pos / 2);
		target_sz = floor(target_sz / 2);
		resize_image = true;
	else
		resize_image = false;
	end
end

