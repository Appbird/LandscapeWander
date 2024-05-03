import cv2
import numpy as np
from sklearn.cluster import KMeans
from PIL import Image

def k_means(img_array:np.ndarray, num_colors, output_path):
    h, w, c = img_array.shape
    img_array = img_array.reshape((h * w, c))

    kmeans = KMeans(n_init='auto', n_clusters=num_colors, random_state=0).fit(img_array)
    centers = np.round(kmeans.cluster_centers_).astype(int)

    img_array = centers[kmeans.labels_]
    img_array = img_array.reshape((h, w, c))
    img = Image.fromarray(img_array.astype('uint8'))
    img.save(output_path)
