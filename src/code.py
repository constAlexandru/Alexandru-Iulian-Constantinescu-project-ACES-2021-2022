import cv2
import numpy as np
import scipy.ndimage as sc
import time

image_name = 'image_cat.png';
image = cv2.imread(image_name);

kernel = np.array([[0, -1, 0], 
                   [-1, 5, -1], 
                   [0, -1, 0]]);
start_time = time.time()
img_filt = cv2.filter2D(image, -1, kernel);
print("--- %s seconds ---" % (time.time() - start_time))
#cv2.imwrite(image_name, img_filt);
print('Done processing image');