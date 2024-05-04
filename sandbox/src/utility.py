import numpy as np
from numpy.typing import NDArray
from cv2.typing import MatLike
from numpy.linalg import norm
from math import atan2

Line = tuple[int, int, int, int]

def arg(line:tuple[int, int, int, int]):
    """
    line = (x1, y1, x2, y2)
    """
    x1 = line[0]
    y1 = line[1]
    x2 = line[2]
    y2 = line[3]
    return atan2(y2 - y1, x2 - x1)

def in_points(line:Line):
    return np.array(
        [line[0], line[1]] if line[0] < line[2] else [line[2], line[3]]
    ), np.array(
        [line[2], line[3]] if line[0] < line[2] else [line[0], line[1]]
    )

def MatLike_to_list_of_points(lines:MatLike) -> list[Line]:
    return list(zip(
        map(int, lines[:, 0, 0]),
        map(int, lines[:, 0, 1]),
        map(int, lines[:, 0, 2]),
        map(int, lines[:, 0, 3])
    ))

def segment_distance(l1:float, r1:float, l2:float, r2:float):
    if (r1 < l1): return segment_distance(r1, l1, l2, r2)
    if (r2 < l2): return segment_distance(l1, r1, r2, l2)
    return max(0.0, max(l1, l2) - min(r2, r1));

def lerp(x:float, a:float, b:float):
    return a/b * (x - b) + a
def lerp_neg(x:float, a:float, b:float):
    return -a/b * (x - b) + a

def clamp(x, min, max):
    if (x < min): return min
    if (x > max): return max
    return x

def cross(p1: NDArray, p2: NDArray):
    return p1[0]*p2[1] - p1[1]*p2[0]

def direction(p1: NDArray, p2: NDArray):
    return (p2 - p1) / norm(p2 - p1)

def normal(p1: NDArray, p2: NDArray):
    p = direction(p1, p2)
    return np.array([-p[1], p[0]])