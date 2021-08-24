#!/usr/local/cs/bin/python3

import sys as s

block_to_data = {} 

inode_to_link_count = {} 

inode_to_ref_count = {}

inode_to_parent = {}

inodeNum_to_parentsNum = {}

ref_inode = {}

rBlocks = [0, 1, 2, 3, 4, 5, 6, 7, 64]
address_lower = 12
address_upper = 27

type_dict = {'s':"SUPERBLOCK", 'b': "BFREE", 'i': "IFREE", 'n':"INODE", 'ind': "INDIRECT", 'dir':"DIRENT"}

glob = {'blocks_count': 0, 'inodes_count': 0, 'offset': 0, 'linkcount': 0, 'links': 0, 'corruption_count': 0}

free_blocks = set()
free_inodes = set()

def error_print(input):
	s.stderr.write(str(input) + '\n')

def check_node_allocation():
	for node in ref_inode.keys():
		snode = str(node)
		s = len(ref_inode[node])
		if node not in free_inodes:
			continue 
		elif node not in inode_to_parent:
			continue
		else:
			print('DIRECTORY INODE ' + str(inode_to_parent[node]) + ' NAME ' + ref_inode[node][:s - 2] + "' UNALLOCATED INODE " + snode)
			glob['corruption_count'] += 1

def check_link_count(link):
	glob['linkcount'] = inode_to_link_count[link]
	if link not in inode_to_ref_count:
		glob['links'] = 0
	else:
		glob['links'] = inode_to_ref_count[link]
	
	slink = str(link)
	if glob['links'] != glob['linkcount']:
		print('INODE ' + slink + ' HAS ' + str(glob['links']) + ' LINKS BUT LINKCOUNT IS ' + str(glob['linkcount']))
		glob['corruption_count'] += 1
	
def check_block(block):
	if block in free_blocks and block in block_to_data.keys():
		glob['corruption_count'] += 1
		print('ALLOCATED BLOCK ' + str(block) + ' ON FREELIST')
	elif block not in free_blocks and block not in block_to_data.keys() and block not in rBlocks:
		glob['corruption_count'] += 1
		print('UNREFERENCED BLOCK ' + str(block))


def check_for_duplicate_columns(block_columns):
	if len(block_to_data[block_columns]) == 1:
		return
	else:
		glob['corruption_count'] += 1
		for ref in block_to_data[block_columns]:
			off = str(ref[1])
			if float(ref[2]) == 0:
				print('DUPLICATE' + ' BLOCK ' + str(block_columns) + ' IN INODE ' + str(ref[0]) + ' AT OFFSET ' + off)
			elif float(ref[2]) == 1:
				print('DUPLICATE INDIRECT' + ' BLOCK ' + str(block_columns) + ' IN INODE ' + str(ref[0]) + ' AT OFFSET ' + off)
			else:
				if float(ref[2]) == 2:
					print('DUPLICATE DOUBLE INDIRECT BLOCK ' + str(block_columns) + ' IN INODE ' + str(ref[0]) + ' AT OFFSET ' + off)
				elif float(ref[2]) == 3:
					print('DUPLICATE TRIPLE INDIRECT BLOCK ' + str(block_columns) + ' IN INODE ' + str(ref[0]) + ' AT OFFSET ' + off)

def inode_allocation_check(inode):
	ten = range(1,11)
	if inode not in free_inodes and inode not in inode_to_link_count:
		if inode not in inode_to_parent and inode not in ten:
			print('UNALLOCATED INODE ' + str(inode) + ' NOT ON FREELIST')
			glob['corruption_count'] += 1
		else:
			pass

	
	elif inode in inode_to_link_count and inode in free_inodes:
		print('ALLOCATED INODE ' + str(inode) + ' ON FREELIST')
		glob['corruption_count'] += 1

def upper_directory_validator(upper):
	if upper == 2:
		print("DIRECTORY INODE 2 NAME '..' LINK TO INODE " + str(inodeNum_to_parentsNum[upper]) + " SHOULD BE 2")
		glob['corruption_count'] += 1
		return
	
	else:
		d_in = ""
		correct_value = ""
		if upper not in inode_to_parent:
			d_in = str(inodeNum_to_parentsNum[upper])
			correct_value = str(inodeNum_to_parentsNum[upper])

		elif inodeNum_to_parentsNum[upper] != inode_to_parent[upper]:
			d_in = str(upper)
			correct_value = str(inode_to_parent[upper])

		if d_in != "":
			glob['corruption_count'] += 1
			print("DIRECTORY INODE " + d_in + " NAME '..' LINK TO INODE " + str(upper) + " SHOULD BE " + correct_value)


def run_audits():
	check_node_allocation()

	for link in inode_to_link_count:
		check_link_count(link)
	
	for upper in inodeNum_to_parentsNum:
		if upper != 2 or inodeNum_to_parentsNum[upper] != 2:
			upper_directory_validator(upper)
	
	num_blocks = glob['blocks_count'] + 1
	for block_number in range(1, num_blocks):
		check_block(block_number)

	num_inodes = glob['inodes_count'] + 1
	for inode in range(1, num_inodes):
		inode_allocation_check(inode)

	for block_columns in block_to_data:
		check_for_duplicate_columns(block_columns)
		
def directory_handler(columns):
	node_num = float(columns[3])

	ref_inode[int(node_num)] = str(columns[6])

	directory = str(columns[6])
	node_parent = float(columns[1])

	if node_num in inode_to_ref_count:
		inode_to_ref_count[int(node_num)] += 1
	else:
		inode_to_ref_count[int(node_num)] = 1

	if node_num >= 2 and node_num <= glob['inodes_count']:
		if "'..'" not in directory and "'.'" not in directory:
			inode_to_parent[int(node_num)] = int(node_parent)
		else: 
			if  "'.'" in directory:
				if node_num == node_parent:
					pass
				else:
					node_num = str(int(node_num))
					node_parent = str(int(node_parent))
					glob['corruption_count'] += 1
					print('DIRECTORY INODE ' + node_parent + " NAME '.' " + "LINK TO INODE " + node_num + ' SHOULD BE ' + node_parent)

			if "'..'" in directory:
				inodeNum_to_parentsNum[int(node_parent)] = int(node_num) 
	else:
		node_num = str(int(node_num))
		node_parent = str(int(node_parent))
		s = len(directory)
		glob['corruption_count'] += 1
		print('DIRECTORY INODE ' + node_parent + ' NAME ' + directory[:s-2] + "' INVALID INODE " + node_num)
		return 0
	
	return 1


def inode_handler(columns):
	node_num = int(columns[1])
	inode_to_link_count[node_num] = int(columns[6])
	for i in range(address_lower, address_upper):
		block_number = int(columns[i])
		if block_number == 0: 
			continue
		indirection_level = ""
		glob['offset'] = 0
		level = 0
		if i == 24:
			indirection_level = " INDIRECT"
			glob['offset'] = 12
			level = 1
		
		elif i == 25:
			indirection_level = " DOUBLE INDIRECT"
			glob['offset'] = 268
			level = 2
		
		elif i == 26:
			indirection_level = " TRIPLE INDIRECT"
			glob['offset'] = 65804
			level = 3

		i_or_r = ""
		if block_number > glob['blocks_count'] or block_number < 0:
			glob['corruption_count'] += 1
			i_or_r = "INVALID"
		
		elif block_number in rBlocks and block_number != 0:
			glob['corruption_count'] += 1
			i_or_r = "RESERVED"
		
		elif block_number in block_to_data:
			block_to_data[block_number].append([node_num, glob['offset'], level])

		else:
			block_to_data[block_number] = [[node_num, glob['offset'], level]]
		
		if i_or_r != "":
			print(i_or_r + indirection_level + ' BLOCK ' + str(block_number) + ' IN INODE ' + str(node_num) + ' AT OFFSET ' + str(glob['offset']))



def indirection_handler(columns):
	block_number = int(columns[5])
	node_num = int(columns[1])

	level = int(columns[2])
	indirection_level = "INDIRECT"
	if level > 3:
		return 0
	elif level == 1:
		glob['offset'] = 12
	elif level == 2:
		indirection_level = "DOUBLE " + indirection_level
		glob['offset'] = 268
	elif level == 3:
		indirection_level = "TRIPLE " + indirection_level
		glob['offset'] = 65804

	i_or_r = ""
	
	if block_number > glob['blocks_count'] or block_number < 0:
		glob['corruption_count'] += 1
		i_or_r = 'INVALID'

	elif block_number in rBlocks:
		glob['corruption_count'] += 1
		i_or_r = 'RESERVED'

	elif block_number not in block_to_data:
		block_to_data[block_number] = [[node_num, glob['offset'], level]]

	else:
		block_to_data[block_number].append([node_num, glob['offset'], level])
	
	if i_or_r != "":
			print(i_or_r + indirection_level + ' BLOCK ' + str(block_number) + ' IN INODE ' + str(node_num) + ' AT OFFSET ' + str(glob['offset']))

def main(input_file):
	lines = input_file.readlines()

	for line in lines:
		columns = line.split(",")
	
		item_type = columns[0]

		if item_type == type_dict['b']: 
			free_blocks.add(int(columns[1])) 

		elif item_type == type_dict['s']:
			glob['blocks_count'] = int(columns[1])
			glob['inodes_count'] = int(columns[2])

		elif item_type == type_dict['i']:
			free_inodes.add(int(columns[1])) 

		elif item_type == type_dict['dir']:
			r = directory_handler(columns)
			if r == 0:
				continue

		elif item_type == type_dict['n']:
			r = inode_handler(columns)
			if r == 0:
				continue
	

		elif item_type == type_dict['ind']:
			r = indirection_handler(columns)
			if r == 0:
				continue


	run_audits()

	if glob['corruption_count'] > 0:
		return 2
	
	return 0

if __name__ ==  "__main__":

	if len(s.argv) != 2:
		error_print("Did not supply the correct number of arguments")
		quit(1)
	try:
		input_file = open(s.argv[1], "r")
	except OSError:
		error_print('file does not exist')
		quit(1)
	r = main(input_file)
	quit(r)