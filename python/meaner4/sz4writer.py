"""
  SZARP: SCADA software 
  Darek Marcinkiewicz <reksio@newterm.pl>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

"""

import sys
sys.path.append("/opt/szarp/lib/python")

import os
import datetime

import meanerbase

class Sz4Writer(meanerbase.MeanerBase):
	def __init__(self, path):
		MeanerBase.__init__(self, path)

	def configure(self, ipk_path):
		MeanerBase.configure(self, path)

		self.name2index = { sp.param.param_name : i for (i, sp) in enumerate(self.save_params) }

	def prepare_value(self, value, param):
		return float(value) * (10 ** param.prec)

	def process_file(self, path, time_format):
		print "Reading values from file %s" % (path,)
		f = fopen(path, 'r')
		
		for line in f:
			(pname, time_string, value_string) = line.rsplit(';', 3)

			pindex = self.name2index[pname]
			if pindex is None:
				print "Param %s not found, skipping line" % (pname,)
				continue
			sparam = self.save_params[pindex]

			_time = time.mktime(time.strptime(time_string, time_format))

			value = self.prepare_value(value_string, sparam.param)
			sparam.process_value(value, _time, 0)

if __name__ == "__main__":
	writer = Sz4Writer(sys.argv[1] + '/sz4')
	writer.configure(sys.argv[1] + '/config/params.xml')

	writer.process_file(sys.argv[2], "%Y-%m-%d %H:%M:%S" if len(sys.argv) == 4 else sys.argv[3])

