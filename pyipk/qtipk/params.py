#!/usr/bin/python
# -*- coding: utf-8 -*-

import sip
sip.setapi('QString', 2)

import string

from PyQt4 import QtGui , QtCore

from lxml import etree

from .utils import * 

from .ui.xml_view import Ui_XmlView

from libipk.params import *

# FIXME: this sux, but dont know yet how to fix it. Without it
# pythons GS deletes objects handle by Qt
globaltab = []

class ModelTree :
	def __init__( self , node , row ) :
		globaltab.append( self )
		
		self.parent = None
		self.node = node 
		self.row  = row

		self.children = None

	def node_changed( self ) :
		pass

	def rows( self ) :
		return len(self.node)

	def fill( self ) :
		if self.children == None :
			self.children = []
			for i in range(len(self.node)) :
				self.add( self.node[i].create_mtnode( i ) )

	def purge( self ) :
		self.children = None

	def remove( self , child ) :
		row = child.row
		self.children.pop(row)
		self.__update_rows(row)

	def __update_rows( self , beg = 0 ) :
		# Update row index.
		# TODO: Could be done faster like this:
		#for i in ragne(beg,len(self.children))
		for i in range(len(self.children)) :
			self.children[i].row = i

	def __update_child( self , child ) :
		child.parent = self
		child.mtmodel = self.mtmodel

	def add( self , child ) :
		self.fill()
		self.children.append( child )
		self.__update_child( child )

	def children_at( self , row ) :
		self.fill()
		return self.children[row]

#    def __repr__( self ) :
#        return unicode( ('ModelTree',self.row,self.node) )

	def delete( self ) :
		''' deletes self from ModelTree structure '''
#        print 'delete' , self , self.mtmodel , self.node
		self.mtmodel.beginRemoveRows(
				self.mtmodel.createIndex(self.row,0,self.parent) , 
				self.row ,
				self.row )
		self.parent.remove( self )
		self.parent = None
		self.mtmodel.endRemoveRows()
		self.node.changedSig.disconnect(self.node_changed)
		self.node. deleteSig.disconnect(self.delete)
		self.node. insertSig.disconnect(self.insert)

	def insert( self , row , factory ) :
		''' inserts new MtNode based on factory function '''
		self.fill()
		self.mtmodel.beginInsertRows(
				self.mtmodel.createIndex(row,0,self) , 
				row ,
				row )
#        print 'insert' , self , row , len(self.children)
		child = factory( row )
		self.children.insert( row , child )
		self.__update_child( child )
		self.__update_rows( row )
		self.mtmodel.endInsertRows()


class QNode( PNode , QtCore.QObject ) :
	changedSig = QtCore.pyqtSignal()
	deleteSig  = QtCore.pyqtSignal()
	insertSig  = QtCore.pyqtSignal(int,object)

	drag = []

	def __init__( self , doc , parent , node ) :
		PNode.__init__( self , doc , parent , node )
		QtCore.QObject.__init__( self )

	def create_mtnode( self , row , mtmodel = None ) :
		''' factory for ModelTree nodes based on self node '''
#        print 'create mt' , self
		mt = ModelTree( self , row ) 
		self.changedSig.connect( mt.node_changed )
		self. deleteSig.connect( mt.delete )
		self. insertSig.connect( mt.insert )
		if mtmodel != None : mt.mtmodel = mtmodel
		return mt

	def touch( self ) :
		self.changedSig.emit()
		PNode.touch( self )

	def delete( self ) :
#        print 'delsig' , self , self.parent
		self.deleteSig.emit()
		PNode.remove( self.parent , self )

	def insert( self , row , child ) :
		self.insertSig.emit( row , child.create_mtnode )
		PNode.insert( self , row , child )
#        print 'insert' , self , self.parent , child , child.parent

	@classmethod
	def add_drag( cls , qnode ) :
		cls.drag.append( qnode )

	@classmethod
	def clear_drag( cls ) :
		del cls.drag[:]

	@classmethod
	def list_drag( cls ) :
		return self.drag

	def __repr__( self ) :
		return unicode( ('QNode',self.node) )


