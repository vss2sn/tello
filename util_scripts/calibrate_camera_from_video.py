#!/usr/bin/env python

import argparse
import cv2
import numpy as np
import yaml

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Calibrate camera using a video of a chessboard')
    parser.add_argument('-i', '--input', help='input video file')
    parser.add_argument('-o', '--output', help='output calibration yaml file')
    parser.add_argument('-c', '--corners', help='output corners file', default=None)
    parser.add_argument('-fs', '--framestep', help='use every nth frame in the video', default=20, type=int)
    parser.add_argument('-ss', '--squaresize', help='length of each square', default=1, type=int)
    args = parser.parse_args()

    criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001)
    video = cv2.VideoCapture(args.input)

    pattern_size = (9, 6)
    objp = np.zeros((np.prod(pattern_size), 3), np.float32)
    objp[:, :2] = np.indices(pattern_size).T.reshape(-1, 2) * args.squaresize

    objpoints = []
    imgpoints = []
    i = -1
    while True:
        i += 1
        retval, img = video.read()
        if not retval:
            break
        if i % args.framestep != 0:
            continue

        print('Frame ' + str(i) + ': Corners ' , end=''),
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        ret, corners = cv2.findChessboardCorners(gray, pattern_size, flags=cv2.CALIB_CB_FILTER_QUADS)
        if ret == True:
            print('found')
            objpoints.append(objp)
            corners2 = cv2.cornerSubPix(gray, corners, (5, 5), (-1, -1), criteria)
            imgpoints.append(corners2)
            img = cv2.drawChessboardCorners(img, pattern_size, corners2,ret)
            cv2.imshow('img',img)
            cv2.waitKey(1)
        else:
            print('not found')
            continue

    cv2.destroyAllWindows()

    print('\nPerforming calibration...')
    ret, mtx, dist, rvecs, tvecs = cv2.calibrateCamera(objpoints, imgpoints, gray.shape[::-1], None, None)
    print("RMS of reprojection error: ", ret)
    print("Camera matrix:\n", mtx)
    print("Distortion coefficients: ", dist)

    calibration = {'RMS of reprojection error': ret, 'Camera matrix': mtx.tolist(), 'Distortion coefficients': dist.tolist() }
    with open(args.output, 'w') as fw:
        yaml.dump(calibration, fw)
