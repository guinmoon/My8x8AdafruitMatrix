from PIL import Image, ImageSequence
from numpy import asarray
# load the image
image = Image.open('./imgs/chupakabra8x8.gif')
# convert image to numpy array
# data = asarray(image)
print("")
pixels_count = 0
a_frames = []
for im_frame in ImageSequence.Iterator(image):
    # Converting it to RGB to ensure that it has 3 dimensions as requested
    im_frame = im_frame.convert('RGB')
    a_frames.append(asarray(im_frame))

matrix_data = ""

for data in a_frames:
    matrix_data += "|"
    for y in range(0, len(data)):
        line = data[y]
        for x in range(0, len(line)):
            pixel = line[x]
            if pixel[0] != 0 or pixel[1] != 0 or pixel[2] != 0:
                new_pix = "%s%s%s%s%s" % (format(x, '01x'), format(y, '01x'), format(
                    pixel[0], '02x'), format(pixel[1], '02x'), format(pixel[2], '02x'))
                matrix_data += new_pix
                pixels_count += 1


print(matrix_data)
print("\n")
# print("%i %i %i"%(pixel[rgb]))
# print(type(data))
# # summarize shape
# print(data.shape)

# # create Pillow image
# image2 = Image.fromarray(data)
# print(type(image2))

# # summarize image details
# print(image2.mode)
# print(image2.size)
