.. trouble:

================================================================================
Troubleshooting
================================================================================

For issues with X11/gdk/pangolin crashes
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    - Undo the patch applied by the install script/cmake for OpenVSLAM (git apply -R <patch>)
    - In videosocket.cpp, comment out:

    .. code-block:: bash

      cv::namedWindow("frame");

    and

    .. code-block:: bash

      {
        std::unique_lock<std::mutex> lk(api_->getMutex());
        cv::imshow("Pilot view", mat.clone());
        cv::waitKey(1);
      }

    The patch is a workaround to allow multiple threads to use ``cv::imshow``, which is not generally supported by OpenCV.

.. note:
  Mac users: As X11 support was removed in the last OS update, this might be a required step.


Response does not match command
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Due to the asynchronous nature of the communication, the responses printed to the command might not be to the command state in the statement (for example in case the joystick was moved after a land command was sent, the statement would read `received response ok to command rc a b c d` instead of `received response ok to command land`)
