# Stereo-Matching
execute run the command as follows:
* ./OpenCV_naive_stereo.exe IMG1_PATH IMG2_PATH OUTPUT WINDOW_SIZE WEIGHT

window size = n | window size = m | window size = l
:: | :: | ::
![naive](https://github.com/SohilZidan/Stereo-Matching/blob/master/data/output_naiveParams%20windsize3-occweight1000.000000.png) | ![naive](https://github.com/SohilZidan/Stereo-Matching/blob/master/data/output_naiveParams%20windsize3-occweight1000.000000.png) | ![naive](https://github.com/SohilZidan/Stereo-Matching/blob/master/data/output_naiveParams%20windsize3-occweight1000.000000.png)
## Disparity Images
### Naive Approach
* Window Size = 3
![naive](https://github.com/SohilZidan/Stereo-Matching/blob/master/data/output_naiveParams%20windsize3-occweight1000.000000.png)

* Window Size = 5
![naive](https://github.com/SohilZidan/Stereo-Matching/blob/master/data/output_naiveParams%20windsize5-occweight1500.000000.png)

* Window Size = 7
![naive](https://github.com/SohilZidan/Stereo-Matching/blob/master/data/output_naiveParams%20windsize7-occweight1500.000000.png)

* Window Size = 9
![naive](https://github.com/SohilZidan/Stereo-Matching/blob/master/data/output_naiveParams%20windsize9-occweight1500.000000.png)


### Dynamic Programming
* Window Size = 3

occweight = 500
![DP](https://github.com/SohilZidan/Stereo-Matching/blob/master/data/output_DP_leftParams%20windsize3-occweight500.000000.png)
occweight = 900
![DP](https://github.com/SohilZidan/Stereo-Matching/blob/master/data/output_DP_leftParams%20windsize3-occweight900.000000.png)
occweight = 1000
![DP](https://github.com/SohilZidan/Stereo-Matching/blob/master/data/output_DP_leftParams%20windsize3-occweight1000.000000.png)
occweight = 1500
![DP](https://github.com/SohilZidan/Stereo-Matching/blob/master/data/output_DP_leftParams%20windsize3-occweight1500.000000.png)
occweight = 2000
![DP](https://github.com/SohilZidan/Stereo-Matching/blob/master/data/output_DP_leftParams%20windsize3-occweight2000.000000.png)

* Window Size = 5

occweight = 1500
![DP](https://github.com/SohilZidan/Stereo-Matching/blob/master/data/output_DP_leftParams%20windsize5-occweight1500.000000.png)
occweight = 2000
![DP](https://github.com/SohilZidan/Stereo-Matching/blob/master/data/output_DP_leftParams%20windsize5-occweight2000.000000.png)
occweight = 2500
![DP](https://github.com/SohilZidan/Stereo-Matching/blob/master/data/output_DP_leftParams%20windsize5-occweight2500.000000.png)

* Window Size = 7

occweight = 1500
![DP](https://github.com/SohilZidan/Stereo-Matching/blob/master/data/output_DP_leftParams%20windsize7-occweight1500.000000.png)
occweight = 2000
![DP](https://github.com/SohilZidan/Stereo-Matching/blob/master/data/output_DP_leftParams%20windsize7-occweight2000.000000.png)
occweight = 2500
![DP](https://github.com/SohilZidan/Stereo-Matching/blob/master/data/output_DP_leftParams%20windsize7-occweight2500.000000.png)

* Window Size = 9

occweight = 1500
![DP](https://github.com/SohilZidan/Stereo-Matching/blob/master/data/output_DP_leftParams%20windsize3-occweight1500.000000.png)
occweight = 2000
![DP](https://github.com/SohilZidan/Stereo-Matching/blob/master/data/output_DP_leftParams%20windsize3-occweight2000.000000.png)

## Cloud Images
* Dynamic Programming Approach

window size = 3, occweight = 900
![DP-3-900](https://github.com/SohilZidan/Stereo-Matching/blob/master/data/snapshot00.png)
