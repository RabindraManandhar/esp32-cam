from fastapi import FastAPI, Request, HTTPException
from fastapi.responses import (
    FileResponse,
    StreamingResponse,
)  # FileResponse for single file, StreamResponse for zipfile
from datetime import datetime
import os
import zipfile
from io import BytesIO


app = FastAPI()

# Directory to save uploaded images
upload_dir = "uploaded_images"
os.makedirs(upload_dir, exist_ok=True)


@app.post("/upload")
async def upload_image(request: Request):
    try:
        # Read image data from the request
        image_file = await request.body()

        if not image_file:
            raise HTTPException(status_code=400, detail="No image provided")

        # Generate a timestamped filename
        timestamp = datetime.now().strftime("%Y%m%d%H%M%S")
        filename = os.path.join(upload_dir, f"image_{timestamp}.jpg")

        # Save the image to the directory
        with open(filename, "wb") as f:
            f.write(image_file)

        return {"message": "Image received", "filename": {filename}}

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))


@app.get("/list_images")
def list_images():
    """Returns a list of all image filenames in the upload directory."""
    try:
        files = os.listdir(upload_dir)

        # Filter only image files (JPG, PNG, etc.)
        image_files = [
            file
            for file in files
            if file.lower().endswith((".png", ".jpg", ".jpeg", ".gif", ".bmp"))
        ]

        if not image_files:
            return {"message": "No images found", "files": []}

        return {"message": "Images retrieved successfully", "files": image_files}

    except Exception as e:
        print(f"Unexpected Server Error: {str(e)}")  # Log the error
        raise HTTPException(status_code=500, detail="Internal Server Error")


@app.get("/get_image/{filename}")
def get_image(filename: str):
    try:
        file_path = os.path.join(upload_dir, filename)

        if not os.path.isfile(file_path):
            print("Error: No images found")  # Log the error
            raise HTTPException(status_code=404, detail="Image not found!")

        return FileResponse(file_path)

    except HTTPException as http_err:
        raise http_err  # Pass through known errors like 404

    except Exception as e:
        print(f"Unexpected Server Error: {str(e)}")  # Log unexpected errors
        raise HTTPException(status_code=500, detail="Internal Server Error")


@app.get("/get_all_images")
def get_all_images():
    try:
        files = os.listdir(upload_dir)

        # If no images exist, return 404 **before attempting to create ZIP**
        if not files:
            print("Error: No images found")  # Log the error
            raise HTTPException(status_code=404, detail="No images found")

        # Create an in-memory zip file
        zip_buffer = BytesIO()
        with zipfile.ZipFile(zip_buffer, "w", zipfile.ZIP_DEFLATED) as zip_file:
            for file in files:
                file_path = os.path.join(upload_dir, file)
                if os.path.isfile(file_path):
                    zip_file.write(file_path, file)

        # If the ZIP file is still empty, return a 404
        if zip_buffer.tell() == 0:
            raise HTTPException(status_code=404, detail="No images found")

        zip_buffer.seek(0)

        return StreamingResponse(
            zip_buffer,
            media_type="application/x-zip-compressed",
            headers={"Content-Disposition": "attachment; filename=images.zip"},
        )

    except HTTPException as http_err:
        raise http_err  # Pass through known errors like 404

    except Exception as e:
        print(f"Unexpected Server Error: {str(e)}")  # Log unexpected errors
        raise HTTPException(status_code=500, detail="Internal Server Error")
