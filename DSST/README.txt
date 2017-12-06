#这是原作者的代码，一点都没改
#核心代码是dsst.m 和get_scale_samples.m和get_translation_sample.m
#这三个核心代码我加了详细的注释
#matlab 2015b测试通过

This MATLAB code implements the Discriminative Scale Space Tracker (DSST) [1]. The implementation is built upon the code provided by [2]. The code provided by [3] is used for computing the HOG features.

The project website is:
http://www.cvl.isy.liu.se/research/objrec/visualtracking/index.html

Installation:
To be able to run the "mexResize" function, try to use either one of the included mex-files or compile one of your own. OpenCV is needed for this. The existing compile scripts "compilemex.m" or "compilemex_win.m" can be modified for this purpose.

Instructions:
1) Run the "run_tracker.m" script in MATLAB.
2) Choose sequence (only "dog1" is included).

Contact:
Martin Danelljan
martin.danelljan@liu.se


[1] Martin Danelljan, Gustav Häger, Fahad Shahbaz Khan and Michael Felsberg.
    "Accurate Scale Estimation for Robust Visual Tracking".
    Proceedings of the British Machine Vision Conference (BMVC), 2014.

[2] J. Henriques, R. Caseiro, P. Martins, and J. Batista.
    "Exploiting the circulant structure of tracking-by-detection with kernels."
    In ECCV, 2012.

[3] Piotr Dollár.
    "Piotr’s Image and Video Matlab Toolbox (PMT)."
    http://vision.ucsd.edu/~pdollar/toolbox/doc/index.html.
