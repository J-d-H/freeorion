import sys
import math
import random


error_list = []


def seed_rng(seed):
    """
    Seeds the default RNG with the specified seed and performs a jumpahead
    """
    random.seed(seed)
    random.jumpahead(999999)


def distance(x1, y1, x2, y2):
    """
    Calculates linear distance between two coordinates.
    """
    return math.hypot(float(x1) - float(x2), float(y1) - float(y2))


def load_string_list(file_name):
    """
    Reads a list of strings from a content file.
    """
    try:
        with open(file_name, "r") as f:
            return [x.strip('" \t\r\n') for x in f]
    except:
        report_error("Python load_string_list: unable to access %s\n" % file_name + sys.exc_info()[1])
        return []


def report_error(msg):
    """
    Handles error messages.
    """
    error_list.append(msg)
    print >> sys.stderr, msg
