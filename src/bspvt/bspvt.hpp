
#ifndef _BSPVT_HPP_
#define _BSPVT_HPP_

namespace vcl {

namespace bspvt {

//BSPVTのノードクラス
template< typename T > class node
{
public:
	//コンストラクタ
	node() : m_child(), m_data(), m_param()
	{

	}

	bool is_leaf() const
	{
		return ( m_child[ 0 ] == nullptr && m_child[ 1 ] == nullptr );
	}

	std::unique_ptr< node > m_child[ 2 ]; //子ノードへのポインタ
	T                       m_data; //現在のクラスタの可視関数の平均値
	T                       m_param[ 2 ]; //分割線の角度¥thetaと切片c
};

template< typename T > class tree
{
public:
	//コンストラクタ
	tree() = default;

	//ファイルからデータ読み込み
	void load( std::ifstream& fin )
	{
		m_root = std::make_unique< node< T > >();
		load_node( m_root, fin, 0 );
	}

	//
	void load_node( const std::unique_ptr< node< T > >& node, std::ifstream& fin, const uint8_t& level )
	{
		char type;
		fin.read( ( char* ) &type, sizeof( char ) );
		if( type == 0 ) {
			node->m_data = 0.f;
			m_nv0++;
			m_depth = std::max( level, m_depth );
		} else if( type == 1 ) {
			node->m_data = 1.f;
			m_nv1++;
			m_depth = std::max( level, m_depth );
		} else if( type == 2 ) {
			fin.read( ( char* ) &node->m_data, sizeof( T ) );
			m_nvx++;
			m_depth = std::max( level, m_depth );
		} else {
			fin.read( ( char* ) node->m_param, 2 * sizeof( T ) );
			node->m_child[ 0 ] = std::make_unique< vcl::bspvt::node< T > >();
			node->m_child[ 1 ] = std::make_unique< vcl::bspvt::node< T > >();
			load_node( node->m_child[ 0 ], fin, level + 1 );
			load_node( node->m_child[ 1 ], fin, level + 1 );
		}
	}

	const std::unique_ptr< node< T > >& root() const { return m_root; }
	size_t              v0() const { return m_nv0; }
	size_t              v1() const { return m_nv1; }
	size_t              vx() const { return m_nvx; }
	uint8_t             depth() const { return m_depth; }

private:

	std::unique_ptr< node< T > >    m_root;
	size_t          m_nv0   = {};
	size_t          m_nv1   = {};
	size_t          m_nvx   = {};
	uint8_t         m_depth = {};

};



}

}

#endif //_BSPVT_HPP_
