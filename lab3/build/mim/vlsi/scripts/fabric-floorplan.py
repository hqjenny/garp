import argparse
parser = argparse.ArgumentParser(description='Generate Fix MIM floorplan for 2d fabric')
parser.add_argument('cols', type=int, help='number of columns')
parser.add_argument('rows', type=int, help='number of rows')
parser.add_argument('tile_x', type=int, help='x-size of a tile')
parser.add_argument('tile_y', type=int, help='y-size of a tile')
parser.add_argument('spacing', type=int, help='inter-tile spacing')

args = parser.parse_args()

fp_cmd = ""
mim_cmd = ""

t = args.spacing

for y in range(0, args.cols):
	l = args.spacing
	for x in range(0, args.rows):
		tile_id = y * args.rows + x
		r = l + args.tile_x
		b = t + args.tile_y
		mim_cmd += ("""set_object_boundary Fix3Stage_%s -bbox "%i %i %i %i"\n""" %
			(tile_id, l, t, r, b))
		l = r + args.spacing
	t = b + args.spacing

fp_cmd = """
  -core_width %i \\
  -core_height %i \\
""" % (l, t)

print mim_cmd
print fp_cmd
