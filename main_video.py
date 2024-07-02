import cv2
import sys
import threading
from simple_facerec import SimpleFacerec
import time
import numpy as np
import os

# ESP32 Server URL
# esp32_server_url = "http://esp32-server-ip/api/store-name"  # Replace with the actual IP address and endpoint

# Encode faces from a folder
sfr = SimpleFacerec()
sfr.load_encoding_images("image/")

# Load Camera
cap = cv2.VideoCapture(0)
cap.set(cv2.CAP_PROP_FPS, 520)

detected_name = None
start_time = None
overlay_start_time = None

# Load your logo image with a transparent background (PNG format)
logo_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "logo.png")
logo = cv2.imread(logo_path, cv2.IMREAD_UNCHANGED)

# Resize the logo to half of its original size
logo = cv2.resize(logo, (0, 0), fx=0.5, fy=0.5)

def display_overlay(frame, name):
    overlay = frame.copy()
    cv2.rectangle(overlay, (0, 0), (frame.shape[1], frame.shape[0]), (0, 255, 0), -1)
    cv2.addWeighted(overlay, 0.45, frame, 0.55, 0, frame)

    # Customize the font properties
    font = cv2.FONT_HERSHEY_SIMPLEX
    font_size = 2
    font_thickness = 3
    font_color = (255, 255, 255)

    # Calculate text size and position for centering
    text_size = cv2.getTextSize(f"Hi, {name}", font, font_size, font_thickness)[0]
    text_x = int((frame.shape[1] - text_size[0]) / 2)
    text_y = int((frame.shape[0] + text_size[1]) / 2)

    # Put the text on the frame
    cv2.putText(frame, f"Hi, {name}", (text_x, text_y), font, font_size, font_color, font_thickness)

    # Calculate logo position centered under the text and moved up
    logo_x = int((frame.shape[1] - logo.shape[1]) / 2)
    logo_y = text_y - int(logo.shape[0] * 0.2)  # Move up by 20% of the logo height

    # Ensure the logo fits without resizing
    logo_resized = logo.copy()
    logo_resized = logo_resized[:min(logo_resized.shape[0], frame.shape[0] - logo_y),
                                 :min(logo_resized.shape[1], frame.shape[1] - logo_x)]

    # Blend the logo with the frame
    for c in range(0, 3):
        frame[logo_y:logo_y + logo_resized.shape[0], logo_x:logo_x + logo_resized.shape[1], c] = \
            frame[logo_y:logo_y + logo_resized.shape[0], logo_x:logo_x + logo_resized.shape[1], c] * \
            (1 - logo_resized[:, :, 3] / 255.0) + \
            logo_resized[:, :, c] * (logo_resized[:, :, 3] / 255.0)

def face_recognition_thread():
    global detected_name
    global start_time
    global overlay_start_time

    while True:
        ret, frame = cap.read()
        if frame is None or frame.size == 0:
            continue

        # Detect Faces
        face_locations, face_names = sfr.detect_known_faces(frame)

        if len(face_locations) > 0:
            # Face detected, start the timer
            if start_time is None:
                start_time = time.time()

            # Update the detected name
            detected_name = face_names[0]

            # Check if the user has been in front of the camera for 5 seconds
            if time.time() - start_time > 5:
                overlay_start_time = time.time()
                start_time = None  # Reset the timer

        if overlay_start_time is not None and (time.time() - overlay_start_time) < 5:
            display_overlay(frame, detected_name)

        cv2.imshow("Frame", frame)

        # Handle events and allow GUI to be responsive
        key = cv2.waitKey(1)
        if key == 27:
            break

# Start the face recognition thread
face_recognition_thread = threading.Thread(target=face_recognition_thread)
face_recognition_thread.start()

# Wait for the thread to finish
face_recognition_thread.join()

# Release the camera
cap.release()
cv2.destroyAllWindows()
