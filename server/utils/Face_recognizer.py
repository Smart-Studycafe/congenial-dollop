import cv2
import numpy as np
import base64
import json

class Face_recognizer:
    def detect_faces(encoded_img):
        # image decoding
        decoded_img = base64.b64decode(encoded_img)
        np_data = np.frombuffer(decoded_img, np.uint8)
        image = cv2.imdecode(np_data, cv2.IMREAD_COLOR)

        # face detection
        gray=cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        face_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_frontalface_default.xml')
        faces = face_cascade.detectMultiScale(gray, scaleFactor=1.05, minNeighbors=6, minSize=(30,30))
        if(len(faces) > 0):
            return True
        else :
            return False
