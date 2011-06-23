#include <stdio.h>
#include <QVariant>
#include "dump.h"

typedef struct {
	int		tag ;
	int		length ;
} RECORDHEADER ;

typedef struct {
	unsigned char	ActionCode ;
	unsigned short	length ;
} ACTIONRECORDHEADER ;

class CSwfDump
{
public:
	enum {
		ClipEventKeyUp			= 0x00000001,
		ClipEventKeyDown		= 0x00000002,
		ClipEventMouseUp		= 0x00000004,
		ClipEventMouseDown		= 0x00000008,
		ClipEventMouseMove		= 0x00000010,
		ClipEventUnload			= 0x00000020,
		ClipEventEnterFrame		= 0x00000040,
		ClipEventLoad			= 0x00000080,
		ClipEventDragOver		= 0x00000100,
		ClipEventRollOut		= 0x00000200,
		ClipEventRollOver		= 0x00000400,
		ClipEventReleaseOutside	= 0x00000800,
		ClipEventRelease		= 0x00001000,
		ClipEventPress			= 0x00002000,
		ClipEventInitialize		= 0x00004000,
		ClipEventData			= 0x00008000,
		ClipEventConstruct		= 0x00010000,
		ClipEventKeyPress		= 0x00020000,
		ClipEventDragOut		= 0x00040000
	} ;

public:
	CSwfDump(char *p) ;
	~CSwfDump() {}

	void print_header() ;
	void read_tags() ;
	void print_rect() ;
	void print_rgb() ;
	void print_rgba() ;
	void print_matrix() ;
	void print_gradient() ;
	void print_grad_record() ;
	void print_shape_records(int fillBits, int lineBits) ;
	void print_focal_gradient() ;
	void print_shapeWithStyle() ;
	void print_fill_style_array() ;
	void print_line_style_array() ;
	void print_fill_style() ;
	void print_line_style() ;
	void print_cxFormWithAlpha() ;
	void print_clip_actions() ;
	int print_clip_event_flags() ;
	void print_clip_action_records() ;
	void print_action_record(unsigned int recordSize) ;

	void print_ActionGotoFrame(ACTIONRECORDHEADER &header) ;
	void print_ActionGetURL(ACTIONRECORDHEADER &header) ;
	void print_ActionWaitForFrame(ACTIONRECORDHEADER &header) ;
	void print_ActionSetTarget(ACTIONRECORDHEADER &header) ;
	void print_ActionGoToLabel(ACTIONRECORDHEADER &header) ;

	int getBytes(int byte) ;
	int getBits(int bits, bool bSign = false) ;
	int getEncodedU32() ;
	QString getString() ;

	void print_file_attributes(RECORDHEADER &tag) ;
	void print_metadata(RECORDHEADER &tag) ;
	void print_back_ground_color(RECORDHEADER &tag) ;
	void print_DefineSceneAndFrameLabelData(RECORDHEADER &tag) ;
	void print_DefineBitsJPEG3(RECORDHEADER &tag) ;
	void print_DefineShape2(RECORDHEADER &tag) ;
	void print_PlaceObject2(RECORDHEADER &tag) ;
	void print_DefineShape(RECORDHEADER &tag) ;
	void print_DefineSprite(RECORDHEADER &tag) ;
	void print_RemoveObject2(RECORDHEADER &tag) ;

	bool getTag(RECORDHEADER &tag) ;
	ACTIONRECORDHEADER getActionRecordHeader() ;

	void start()
	{
		strDump = "dump start --------------\n" ;
	}
	void end()
	{
		strDump += "dump end\n" ;
	}
	QString getStr() { return strDump ; }

private:
	char			*pTop ;
	unsigned int	offset ;
	unsigned int	bit_offset ;
	int				currentShape ;
	int				version ;
	bool			bDefineSprite ;
	QString			strDump ;
} ;

CSwfDump::CSwfDump(char *p)
{
	pTop = p ;
	offset = 0 ;
	bit_offset = 0 ;
	currentShape = 0 ;
	version = 0 ;
	bDefineSprite = false ;
}

void CSwfDump::print_header()
{
	strDump += "header -----------------\n" ;
	char m0 = getBytes(1) ;
	char m1 = getBytes(1) ;
	char m2 = getBytes(1) ;
	strDump += QString("magic:%1%2%3\n").arg(m0).arg(m1).arg(m2) ;
	version = getBytes(1) ;
	strDump += QString("version:%1\n").arg(version, 0, 16) ;
	strDump += QString("file size:%1\n").arg(getBytes(4)) ;
	print_rect() ;
	strDump += QString("frame rate:%1\n").arg(getBytes(2), 0, 16) ;
	strDump += QString("frame count:%1\n").arg(getBytes(2)) ;
}

void CSwfDump::read_tags()
{
	RECORDHEADER tag ;
	while ( getTag(tag) ) {
		if ( tag.length == 0x3f ) {
			tag.length = getBytes(4) ;
		}
		strDump += QString("tag[%1] offset[0x%2] ").arg(tag.tag).arg(offset, 0, 16) ;
		switch ( tag.tag ) {
		case 0:	// END
			strDump += "\n" ;
			if ( bDefineSprite ) {
				bDefineSprite = false ;
				break ;
			}
			return ;
		case 1:
			strDump += "ShowFrame\n" ;
			break ;
		case 2:
			print_DefineShape(tag) ;
			break ;
		case 9:
			print_back_ground_color(tag) ;
			break ;
		case 22:
			print_DefineShape2(tag) ;
			break ;
		case 26:
			print_PlaceObject2(tag) ;
			break ;
		case 28:
			print_RemoveObject2(tag) ;
			break ;
		case 35:
			print_DefineBitsJPEG3(tag) ;
			break ;
		case 39:
			print_DefineSprite(tag) ;
			break ;
		case 69:
			print_file_attributes(tag) ;
			break ;
		case 77:
			print_metadata(tag) ;
			break ;
		case 86:
			print_DefineSceneAndFrameLabelData(tag) ;
			break ;
		default:
			qDebug("undump tag:%d len:%d", tag.tag, tag.length) ;
			return ;
		}
	}
}

void CSwfDump::print_rect()
{
	int bits = getBits(5) ;
	strDump += "RECT ------------------\n" ;
	strDump += QString("bits:%1\n").arg(bits) ;
	strDump += QString("Xmin:%1\n").arg(getBits(bits, true)) ;
	strDump += QString("Xmax:%1\n").arg(getBits(bits, true)) ;
	strDump += QString("Ymin:%1\n").arg(getBits(bits, true)) ;
	strDump += QString("Ymax:%1\n").arg(getBits(bits, true)) ;
}

void CSwfDump::print_rgb()
{
	int r = getBytes(1) ;
	int g = getBytes(1) ;
	int b = getBytes(1) ;
	strDump += QString("r:%1 g:%2 b:%3\n").arg(r).arg(g).arg(b) ;
}
void CSwfDump::print_rgba()
{
	int r = getBytes(1) ;
	int g = getBytes(1) ;
	int b = getBytes(1) ;
	int a = getBytes(1) ;
	strDump += QString("r:%1 g:%2 b:%3 a:%4\n").arg(r).arg(g).arg(b).arg(a) ;
}

void CSwfDump::print_matrix()
{
	int has ;

	strDump += QString("MATRIX [offset:0x%1] ----------------\n").arg(offset, 0, 16) ;
	has = getBits(1) ;
	strDump += QString("HasScale:%1\n").arg(has) ;
	if ( has ) {
		int scaleBit = getBits(5) ;
		int scaX = getBits(scaleBit) ;
		int scaY = getBits(scaleBit) ;
		strDump += QString("NScaleBits:%1\n").arg(scaleBit) ;
		strDump += QString("ScaleX:%1\n").arg(scaX, 0, 16) ;
		strDump += QString("ScaleY:%1\n").arg(scaY, 0, 16) ;
	}

	has = getBits(1) ;
	strDump += QString("HasRotate:%1\n").arg(has) ;
	if ( has ) {
		int bit = getBits(5) ;
		int scaX = getBits(bit) ;
		int scaY = getBits(bit) ;
		strDump += QString("NRotateBits:%1\n").arg(bit) ;
		strDump += QString("RotateSkew0:%1\n").arg(scaX, 0, 16) ;
		strDump += QString("RotateSkew1:%1\n").arg(scaY, 0, 16) ;
	}
	int bit = getBits(5) ;
	int tx = getBits(bit, true) ;
	int ty = getBits(bit, true) ;
	strDump += QString("NTranslateBits:%1\n").arg(bit) ;
	strDump += QString("TranslateX:%1\n").arg(tx) ;
	strDump += QString("TranslateY:%1\n").arg(ty) ;
}

void CSwfDump::print_gradient()
{
	int nGrads = 0 ;

	strDump += "GRADIENT ---------------------\n" ;
	strDump += QString("SpreadMode:%1\n").arg(getBits(2)) ;
	strDump += QString("InterpolationMode:%1\n").arg(getBits(2)) ;
	nGrads = getBits(4) ;
	strDump += QString("NumGradients:%1\n").arg(nGrads) ;
	for ( int i = 0 ; i < nGrads ; i ++ ) {
		strDump += QString("GradientRecords[%1] ").arg(i) ;
		print_grad_record() ;
	}
}

void CSwfDump::print_grad_record()
{
	strDump += "GRADRECORD ------------------\n" ;
	strDump += QString("Ratio:%1\n").arg(getBytes(1)) ;
	switch ( currentShape ) {
	case 1: case 2:
		print_rgb();
		break ;
	case 3:
		print_rgba();
		break ;
	}
}

void CSwfDump::print_shape_records(int fillBits, int lineBits)
{
	while ( 1 ) {
		int type_flag = getBits(1) ;
		switch ( type_flag ) {
		case 0:
		{
			int stateNewStyle = getBits(1) ;
			int stateLineStyle = getBits(1) ;
			int stateFillStyle1 = getBits(1) ;
			int stateFillStyle0 = getBits(1) ;
			int stateMoveTo = getBits(1) ;
			if ( !stateNewStyle && !stateLineStyle && !stateFillStyle1 && !stateFillStyle0 && !stateMoveTo ) {
				strDump += QString("ENDSHAPERECORD [offset:0x%1] -----------------------\n").arg(offset, 0, 16) ;
				return ;
			}
			strDump += QString("STYLECHANGERECORD [offset:0x%1] -----------------------\n").arg(offset, 0, 16) ;
			strDump += QString("StateNewStyles:%1\n").arg(stateNewStyle) ;
			strDump += QString("StateLineStyle:%1\n").arg(stateLineStyle) ;
			strDump += QString("StateFillStyle1:%1\n").arg(stateFillStyle1) ;
			strDump += QString("StateFillStyle0:%1\n").arg(stateFillStyle0) ;
			strDump += QString("StateMoveTo:%1\n").arg(stateMoveTo) ;
			if ( stateMoveTo ) {
				int moveBits = getBits(5) ;
				strDump += QString("MoveBits:%1\n").arg(moveBits) ;
				strDump += QString("MoveDeltaX:%1\n").arg(getBits(moveBits, true)) ;
				strDump += QString("MoveDeltaY:%1\n").arg(getBits(moveBits, true)) ;
			}
			if ( stateFillStyle0 ) {
				strDump += QString("FillStyle0:%1\n").arg(getBits(fillBits)) ;
			}
			if ( stateFillStyle1 ) {
				strDump += QString("FillStyle1:%1\n").arg(getBits(fillBits)) ;
			}
			if ( stateLineStyle ) {
				strDump += QString("LineStyle:%1\n").arg(getBits(lineBits)) ;
			}
			if ( stateNewStyle ) {
				strDump += "FillStyles " ;
				print_fill_style_array();
				strDump += "LineStyles " ;
				print_line_style_array();
				fillBits = getBits(4) ;
				lineBits = getBits(4) ;
				strDump += QString("NumFillBits:%1\n").arg(fillBits) ;
				strDump += QString("NumLineBits:%1\n").arg(lineBits) ;
			}
		}
			break ;
		case 1:	// edge record
		{
			int straightFlag = getBits(1) ;
			if ( straightFlag ) {
				strDump += QString("STRAIGHTEDGERECORD [offset:0x%1] -----------------------\n").arg(offset, 0, 16) ;
				int numBits = getBits(4) ;
				strDump += QString("NumBits:%1\n").arg(numBits) ;
				int generalLineFlag = getBits(1) ;
				int vertLineFlag = 0 ;
				if ( !generalLineFlag ) {
					vertLineFlag = getBits(1) ;
				}
				strDump += QString("GeneralLineFlag:%1\n").arg(generalLineFlag) ;
				strDump += QString("VertLineFlag:%1\n").arg(vertLineFlag) ;
				if ( generalLineFlag || !vertLineFlag ) {
					strDump += QString("DeltaX:%1\n").arg(getBits(numBits+2)) ;
				}
				if ( generalLineFlag || vertLineFlag ) {
					strDump += QString("DeltaY:%1\n").arg(getBits(numBits+2)) ;
				}
			}
			else {
				strDump += "CURVEDEDGERECORD -----------------------\n" ;
				int numBits = getBits(4) ;
				strDump += QString("NumBits:%1\n").arg(numBits) ;
				strDump += QString("ControlDeltaX:%1\n").arg(getBits(numBits+2, true)) ;
				strDump += QString("ControlDeltaY:%1\n").arg(getBits(numBits+2, true)) ;
				strDump += QString("AnchorDeltaX:%1\n").arg(getBits(numBits+2, true)) ;
				strDump += QString("AnchorDeltaY:%1\n").arg(getBits(numBits+2, true)) ;
			}
		}
			break ;
		}
	}
}

void CSwfDump::print_focal_gradient()
{
	int nGrads = 0 ;

	strDump += "FOCALGRADIENT ---------------------\n" ;
	strDump += QString("SpreadMode:%1\n").arg(getBits(2)) ;
	strDump += QString("InterpolationMode:%1\n").arg(getBits(2)) ;
	nGrads = getBits(4) ;
	strDump += QString("NumGradients:%1\n").arg(nGrads) ;
	for ( int i = 0 ; i < nGrads ; i ++ ) {
		strDump += QString("GradientRecords[%1] ").arg(i) ;
		print_grad_record() ;
	}
	strDump += QString("FocalPoint:%1\n").arg(getBytes(2), 0, 16) ;
}

void CSwfDump::print_shapeWithStyle()
{
	strDump += "SHAPEWITHSTYLE ------------------\n" ;
	print_fill_style_array() ;
	print_line_style_array() ;
	int fillBits = getBits(4) ;
	int lineBits = getBits(4) ;
	strDump += QString("NumFillBits:%1\n").arg(fillBits) ;
	strDump += QString("NumLineBits:%1\n").arg(lineBits) ;
	print_shape_records(fillBits, lineBits) ;
}

void CSwfDump::print_fill_style_array()
{
	strDump += "FILLSTYLEARRAY\n" ;
	int count = getBytes(1) ;
	strDump += QString("FillStyleCount:%1\n").arg(count) ;
	if ( count == 0xff ) {
		count = getBytes(2) ;
		strDump += QString("FillStyleCountExtended:%1").arg(count) ;
	}
	for ( int i = 0 ; i < count ; i ++ ) {
		strDump += QString("FillStyle:[%1]\n").arg(i) ;
		print_fill_style() ;
	}
}

void CSwfDump::print_line_style_array()
{
	strDump += QString("LINESTYLEARRAY [offset:0x%1 bit:%2] ------------------\n").arg(offset, 0, 16).arg(bit_offset) ;
	int count = getBytes(1) ;
	strDump += QString("LineStyleCount:%1\n").arg(count) ;
	if ( count == 0xff ) {
		count = getBytes(2) ;
		strDump += QString("LineStyleCountExtended:%1\n").arg(count) ;
	}
	for ( int i = 0 ; i < count ; i ++ ) {
		strDump += QString("LineStyle:[%1]\n").arg(i) ;
		print_line_style() ;
	}
}

void CSwfDump::print_fill_style()
{
	int type = getBytes(1) ;
	strDump += "FILLSTYLE --------------\n" ;
	strDump += QString("FillStyleType:0x%1\n").arg(type, 0, 16) ;
	if ( type == 0 ) {
		switch ( currentShape ) {
		case 1: case 2:
			strDump += "Color " ;
			print_rgb();
			break ;
		case 3:
			strDump += "Color " ;
			print_rgba();
			break ;
		}
	}
	if ( type == 0x10 || type == 0x12 || type == 0x13 ) {
		strDump += "GradientMatrix " ;
		print_matrix();
	}

	if ( type == 0x10 || type == 0x12 ) {
		strDump += "Gradient " ;
		print_gradient() ;
	}
	else if ( type == 0x13 ) {
		strDump += "Gradient " ;
		print_focal_gradient() ;
	}

	if ( type >= 0x40 && type <= 0x43 ) {
		strDump += QString("BitmapId:%1\n").arg(getBytes(2)) ;
		strDump += "BitmapMatrix " ;
		print_matrix();
	}
}

void CSwfDump::print_line_style()
{
	strDump += "LINESTYLE\n" ;
	strDump += QString("Width:%1\n").arg(getBytes(2)) ;
	switch ( currentShape ) {
	case 1: case 2:
		strDump += "Color " ;
		print_rgb();
		break ;
	case 3:
		strDump += "Color " ;
		print_rgba();
		break ;
	}
}

void CSwfDump::print_cxFormWithAlpha()
{
	strDump += "CXFORMWITHALPHA --------------\n" ;
	int hasAddTerms = getBits(1) ;
	int hasMultTerms = getBits(1) ;
	int nBits = getBits(4) ;
	strDump += QString("HasAddTerms:%1\n").arg(hasAddTerms) ;
	strDump += QString("HasMultTerms:%1\n").arg(hasMultTerms) ;
	strDump += QString("Nbits:%1\n").arg(nBits) ;
	if ( hasMultTerms ) {
		strDump += QString("RedMultTerm:%1\n").arg(getBits(nBits, true)) ;
		strDump += QString("GreenMultTerm:%1\n").arg(getBits(nBits, true)) ;
		strDump += QString("BlueMultTerm:%1\n").arg(getBits(nBits, true)) ;
		strDump += QString("AlphaMultTerm:%1\n").arg(getBits(nBits, true)) ;
	}
	if ( hasAddTerms ) {
		strDump += QString("RedAddTerm:%1\n").arg(getBits(nBits, true)) ;
		strDump += QString("GreenAddTerm:%1\n").arg(getBits(nBits, true)) ;
		strDump += QString("BlueAddTerm:%1\n").arg(getBits(nBits, true)) ;
		strDump += QString("AlphaAddTerm:%1\n").arg(getBits(nBits, true)) ;
	}
}

void CSwfDump::print_clip_actions()
{
	strDump += "CLIPACTIONS -------------------\n" ;
	strDump += QString("Reserved:%1\n").arg(getBytes(2)) ;
	strDump += "AllEventFlags " ;
	print_clip_event_flags() ;
	strDump += "ClipActionRecords " ;
	print_clip_action_records() ;
	if ( version <= 5 ) {
		strDump += QString("ClipActionEndFlag:%1\n").arg(getBytes(2)) ;
	}
	else {
		strDump += QString("ClipActionEndFlag:%1\n").arg(getBytes(4)) ;
	}
}

int CSwfDump::print_clip_event_flags()
{
	int ret = 0 ;
	strDump += "CLIPEVENTFLAGS ---------------\n" ;
	if ( getBits(1) ) {	ret |= ClipEventKeyUp ; }
	if ( getBits(1) ) {	ret |= ClipEventKeyDown ; }
	if ( getBits(1) ) {	ret |= ClipEventMouseUp ; }
	if ( getBits(1) ) {	ret |= ClipEventMouseDown ; }
	if ( getBits(1) ) {	ret |= ClipEventMouseMove ; }
	if ( getBits(1) ) {	ret |= ClipEventUnload ; }
	if ( getBits(1) ) {	ret |= ClipEventEnterFrame ; }
	if ( getBits(1) ) {	ret |= ClipEventLoad ; }
	if ( getBits(1) ) {	ret |= ClipEventDragOver ; }
	if ( getBits(1) ) {	ret |= ClipEventRollOut ; }
	if ( getBits(1) ) {	ret |= ClipEventRollOver ; }
	if ( getBits(1) ) {	ret |= ClipEventReleaseOutside ; }
	if ( getBits(1) ) {	ret |= ClipEventRelease ; }
	if ( getBits(1) ) {	ret |= ClipEventPress ; }
	if ( getBits(1) ) {	ret |= ClipEventInitialize ; }
	if ( getBits(1) ) {	ret |= ClipEventData ; }

	strDump += QString("ClipEventKeyUp:%1\n").arg(ret & ClipEventKeyUp) ;
	strDump += QString("ClipEventKeyDown:%1\n").arg(ret & ClipEventKeyDown) ;
	strDump += QString("ClipEventMouseUp:%1\n").arg(ret & ClipEventMouseUp) ;
	strDump += QString("ClipEventMouseDown:%1\n").arg(ret & ClipEventMouseDown) ;
	strDump += QString("ClipEventMouseMove:%1\n").arg(ret & ClipEventMouseMove) ;
	strDump += QString("ClipEventUnload:%1\n").arg(ret & ClipEventUnload) ;
	strDump += QString("ClipEventEnterFrame:%1\n").arg(ret & ClipEventEnterFrame) ;
	strDump += QString("ClipEventLoad:%1\n").arg(ret & ClipEventLoad) ;
	strDump += QString("ClipEventDragOver:%1\n").arg(ret & ClipEventDragOver) ;
	strDump += QString("ClipEventRollOut:%1\n").arg(ret & ClipEventRollOut) ;
	strDump += QString("ClipEventRollOver:%1\n").arg(ret & ClipEventRollOver) ;
	strDump += QString("ClipEventReleaseOutside:%1\n").arg(ret & ClipEventReleaseOutside) ;
	strDump += QString("ClipEventRelease:%1\n").arg(ret & ClipEventRelease) ;
	strDump += QString("ClipEventPress:%1\n").arg(ret & ClipEventPress) ;
	strDump += QString("ClipEventInitialize:%1\n").arg(ret & ClipEventInitialize) ;
	strDump += QString("ClipEventData:%1\n").arg(ret & ClipEventData) ;
	if ( version >= 6 ) {
		strDump += QString("Reserved:%1\n").arg(getBits(5)) ;

		if ( getBits(1) ) {	ret |= ClipEventConstruct ; }
		if ( getBits(1) ) {	ret |= ClipEventKeyPress ; }
		if ( getBits(1) ) {	ret |= ClipEventDragOut ; }

		strDump += QString("ClipEventConstruct:%1\n").arg(ret & ClipEventConstruct) ;
		strDump += QString("ClipEventKeyPress:%1\n").arg(ret & ClipEventKeyPress) ;
		strDump += QString("ClipEventDragOut:%1\n").arg(ret & ClipEventDragOut) ;

		strDump += QString("Reserved:%1\n").arg(getBits(8)) ;
	}
	return ret ;
}

void CSwfDump::print_clip_action_records()
{
	strDump += "CLIPACTIONRECORD --------------\n" ;
	strDump += "EventFlags " ;
	int flags = print_clip_event_flags();
	unsigned int actionRecordSize = getBytes(4) ;
	strDump += QString("ActionRecordSize:%1\n").arg(actionRecordSize) ;
	actionRecordSize += offset ;
	if ( flags & ClipEventKeyPress ) {
		strDump += QString("KeyCode:%1\n").arg(getBytes(1)) ;
		actionRecordSize -- ;
	}
	strDump += "Actions " ;
	print_action_record(actionRecordSize) ;
}

void CSwfDump::print_action_record(unsigned int endOffset)
{
	strDump += "ACTIONRECORD ---------------\n" ;
	while ( 1 ) {
		if ( offset >= endOffset ) { return ; }

		ACTIONRECORDHEADER header = getActionRecordHeader() ;
		strDump += QString("ActionCode:%1 ").arg(header.ActionCode) ;
		switch ( header.ActionCode ) {
		case 0x04:
			strDump += "ActionNextFrame\n" ;
			break ;
		case 0x05:
			strDump += "ActionPreviousFrame\n" ;
			break ;
		case 0x06:
			strDump += "ActionPlay\n" ;
			break ;
		case 0x07:
			strDump += "ActionStop\n" ;
			break ;
		case 0x08:
			strDump += "ActionToggleQuality\n" ;
			break ;
		case 0x09:
			strDump += "ActionStopSounds\n" ;
			break ;
		case 0x81:
			print_ActionGotoFrame(header) ;
			break ;
		case 0x82:
			print_ActionGetURL(header) ;
			break ;
		case 0x8a:
			print_ActionWaitForFrame(header) ;
			break ;
		case 0x8b:
			print_ActionSetTarget(header) ;
			break ;
		case 0x8c:
			print_ActionGoToLabel(header) ;
			break ;
		default:
			strDump += "Unknown Action\n" ;
			offset += header.length ;
			break ;
		}
	}
}

void CSwfDump::print_ActionGotoFrame(ACTIONRECORDHEADER &header)
{
	strDump += "ActionGotoFrame\n" ;
	strDump += QString("Frame:%1\n").arg(getBytes(2)) ;
}

void CSwfDump::print_ActionGetURL(ACTIONRECORDHEADER &header)
{
	strDump += "ActionGetURL\n" ;
	strDump += QString("UrlString:%1\n").arg(getString()) ;
	strDump += QString("TargetString:%1\n").arg(getString()) ;
}

void CSwfDump::print_ActionWaitForFrame(ACTIONRECORDHEADER &header)
{
	strDump += "ActionWaitForFrame\n" ;
	strDump += QString("Frame:%1\n").arg(getBytes(2)) ;
	strDump += QString("SkipCount:%1\n").arg(getBytes(1)) ;
}

void CSwfDump::print_ActionSetTarget(ACTIONRECORDHEADER &header)
{
	strDump += "ActionSetTarget\n" ;
	strDump += QString("TargetName:%1\n").arg(getString()) ;
}

void CSwfDump::print_ActionGoToLabel(ACTIONRECORDHEADER &header)
{
	strDump += "ActionGoToLabel" ;
	strDump += QString("Label:%1\n").arg(getString()) ;
}

int CSwfDump::getBytes(int byte)
{
	int ret = 0 ;
	int count = 0 ;
	if ( byte ) {
		if ( bit_offset ) { offset ++ ; }
		bit_offset = 0 ;
	}

	while ( byte ) {
		ret |= (pTop[offset]&0xff) << (count*8) ;
		offset ++ ;
		count ++ ;
		byte -- ;
	}
	return ret ;
}

int CSwfDump::getBits(int bits, bool bSign)
{
	while ( bit_offset > 7 ) {
		offset ++ ;
		bit_offset -= 8 ;
	}
	int ret = 0 ;
	int bits_orig = bits ;
	while ( bits ) {
		ret = (ret << 1) | ((pTop[offset]>>(7-bit_offset)) & 0x01) ;
		bit_offset ++ ;
		if ( bit_offset > 7 ) {
			bit_offset -= 8 ;
			offset ++ ;
		}
		bits -- ;
	}
	if ( bSign ) {
		bits = bits_orig ;
		qDebug("ret=0x%x bit=%d", ret, bits) ;
		if ( ret & (1 << (bits-1)) ) {
			for ( int i = bits ; i < 32 ; i ++ ) {
				ret |= (1 << i) ;
			}
		}
	}
	return ret ;
}

int CSwfDump::getEncodedU32()
{
	int result = pTop[offset];
	if (!(result & 0x00000080)) {
		offset ++ ;
		return result;
	}
	result = (result & 0x0000007f) | pTop[offset]<<7;
	if (!(result & 0x00004000)) {
		offset += 2 ;
		return result;
	}
	result = (result & 0x00003fff) | pTop[offset]<<14;
	if (!(result & 0x00200000)) {
		offset += 3;
		return result;
	}
	result = (result & 0x001fffff) | pTop[offset]<<21;
	if (!(result & 0x10000000)) {
		offset += 4;
		return result;
	}
	result = (result & 0x0fffffff) | pTop[offset]<<28;
	offset += 5;
	return result;
}

QString CSwfDump::getString()
{
	char c ;
	QString ret ;
	while ( (c = getBytes(1)) ) {
		ret += c ;
	}
	return ret ;
}

bool CSwfDump::getTag(RECORDHEADER &tag)
{
	int b = getBytes(2) ;
	tag.tag = b >> 6 ;
	tag.length = b & 0x3f ;
	return true ;
}

ACTIONRECORDHEADER CSwfDump::getActionRecordHeader()
{
	ACTIONRECORDHEADER ret ;
	ret.ActionCode = getBytes(1) ;
	ret.length = 0 ;
	if ( ret.ActionCode >= 0x80 ) {
		ret.length = getBytes(2) ;
	}
	return ret ;
}

void CSwfDump::print_file_attributes(RECORDHEADER &tag)
{
	strDump += "File Attributes ------------------\n" ;
	strDump += QString("Reserved:%1\n").arg(getBits(1)) ;
	strDump += QString("UseDirectBlit:%1\n").arg(getBits(1)) ;
	strDump += QString("UseGPU:%1\n").arg(getBits(1)) ;
	strDump += QString("HasMetadata:%1\n").arg(getBits(1)) ;
	strDump += QString("ActionScript3:%1\n").arg(getBits(1)) ;
	strDump += QString("Reserved:%1\n").arg(getBits(2)) ;
	strDump += QString("UseNetwork:%1\n").arg(getBits(1)) ;
	strDump += QString("Reserved:%1\n").arg(getBits(24)) ;
}

void CSwfDump::print_metadata(RECORDHEADER &tag)
{
	strDump += "Metadata ------------------\n" ;
	char c ;
	while ( (c = getBytes(1)) ) {
		strDump += c ;
	}
	strDump += "\n" ;
}

void CSwfDump::print_back_ground_color(RECORDHEADER &tag)
{
	strDump += "Background Color ---------------\n" ;
	print_rgb() ;
}

void CSwfDump::print_DefineSceneAndFrameLabelData(RECORDHEADER &tag)
{
	strDump += "DefineSceneAndFrameLabelData ---------------\n" ;
	int count = getEncodedU32() ;
	strDump += QString("SceneCount:%1\n").arg(count) ;
	for ( int i = 0 ; i < count ; i ++ ) {
		strDump += QString("Offset %1:%2\n").arg(i).arg(getEncodedU32()) ;
		strDump += QString("Name %1:%2\n").arg(i).arg(getString()) ;
	}
	count = getEncodedU32() ;
	strDump += QString("FrameLabelCount:%1\n").arg(count) ;
	for ( int i = 0 ; i < count ; i ++ ) {
		strDump += QString("FrameNum %1:%2\n").arg(i).arg(getEncodedU32()) ;
		strDump += QString("FrameLabel %1:%2\n").arg(i).arg(getString()) ;
	}
}

void CSwfDump::print_DefineBitsJPEG3(RECORDHEADER &tag)
{
	int endOffset = offset + tag.length ;

	strDump += "DefineBitsJPEG3 ---------------\n" ;
	strDump += QString("CharacterID:%1\n").arg(getBytes(2)) ;
	unsigned int alpha_offset = getBytes(4) ;
	strDump += QString("AlphaDataOffset:%1\n").arg(alpha_offset) ;
	strDump += "ImageData:\n" ;
	for ( unsigned int i = 0 ; i < alpha_offset ; i ++ ) {
		if ( i && !(i % 16) ) {
			strDump += "\n" ;
		}
		strDump += QString("%1 ").arg(getBytes(1), 2, 16) ;
	}
	strDump += "\n" ;

	strDump += "BitmapAlphaData:\n" ;
	int i = 0 ;
	while ( offset < (unsigned int)endOffset ) {
		if ( i && !(i % 16) ) {
			strDump += "\n" ;
		}
		strDump += QString("%1 ").arg(getBytes(1), 2, 16) ;
		i ++ ;
	}
	strDump += "\n" ;

	offset = endOffset ;
}

void CSwfDump::print_DefineShape2(RECORDHEADER &tag)
{
	currentShape = 2 ;
	strDump += "DefineShape2 ---------------\n" ;
	strDump += QString("ShapeId:%1\n").arg(getBytes(2)) ;
	strDump += "ShapeBounds:" ;
	print_rect();
	strDump += "Shapes:" ;
	print_shapeWithStyle() ;
}

void CSwfDump::print_PlaceObject2(RECORDHEADER &tag)
{
	strDump += "PlaceObject2 ---------------\n" ;
	int placeFlagHasClipActions = getBits(1) ;
	int placeFlagHasClipDepth = getBits(1) ;
	int placeFlagHasName = getBits(1) ;
	int placeFlagHasRatio = getBits(1) ;
	int placeFlagHasolorTransform = getBits(1) ;
	int placeFlagHasMatrix = getBits(1) ;
	int placeFlagHasCharacter = getBits(1) ;
	strDump += QString("PlaceFlagHasClipActions:%1\n").arg(placeFlagHasClipActions) ;
	strDump += QString("PlaceFlagHasClipDepth:%1\n").arg(placeFlagHasClipDepth) ;
	strDump += QString("PlaceFlagHasName:%1\n").arg(placeFlagHasName) ;
	strDump += QString("PlaceFlagHasRatio:%1\n").arg(placeFlagHasRatio) ;
	strDump += QString("PlaceFlagHasColorTransform:%1\n").arg(placeFlagHasolorTransform) ;
	strDump += QString("PlaceFlagHasMatrix:%1\n").arg(placeFlagHasMatrix) ;
	strDump += QString("PlaceFlagHasCharacter:%1\n").arg(placeFlagHasCharacter) ;
	strDump += QString("PlaceFlagMove:%1\n").arg(getBits(1)) ;
	strDump += QString("Depth:%1\n").arg(getBytes(2)) ;
	if ( placeFlagHasCharacter ) {
		strDump += QString("CharacterId:%1\n").arg(getBytes(2)) ;
	}
	if ( placeFlagHasMatrix ) {
		strDump += "Matrix " ;
		print_matrix();
	}
	if ( placeFlagHasolorTransform ) {
		strDump += "ColorTransform " ;
		print_cxFormWithAlpha() ;
	}
	if ( placeFlagHasRatio ) {
		strDump += QString("Ratio:%1\n").arg(getBytes(2)) ;
	}
	if ( placeFlagHasName ) {
		strDump += QString("Name:%1\n").arg(getString()) ;
	}
	if ( placeFlagHasClipDepth ) {
		strDump += QString("ClipDepth:%1\n").arg(getBytes(2)) ;
	}
	if ( placeFlagHasClipActions ) {
		strDump += "ClipActions " ;
		print_clip_actions() ;
	}
}

void CSwfDump::print_DefineShape(RECORDHEADER &tag)
{
	strDump += "DefineShape ----------------\n" ;
	strDump += QString("SpaheId:%1\n").arg(getBytes(2)) ;
	strDump += "ShapeBounds:" ;
	print_rect();
	strDump += "Shapes " ;
	print_shapeWithStyle();
}

void CSwfDump::print_DefineSprite(RECORDHEADER &tag)
{
	bDefineSprite = true ;
	strDump += "DefineSprite ----------------\n" ;
	strDump += QString("Sprite ID:%1\n").arg(getBytes(2)) ;
	strDump += QString("FrameCount:%1\n").arg(getBytes(2)) ;
}

void CSwfDump::print_RemoveObject2(RECORDHEADER &tag)
{
	strDump += "RemoveObject2 -----------\n" ;
	strDump += QString("Depth:%1\n").arg(getBytes(2)) ;
}

QString dump(QByteArray &data)
{
	char *p = data.data() ;
	if ( !p ) {
		printf("data not found") ;
		return QString() ;
	}
	CSwfDump dump(p) ;

	dump.start() ;
	dump.print_header() ;
	dump.read_tags() ;
	dump.end() ;
	return dump.getStr() ;
}
