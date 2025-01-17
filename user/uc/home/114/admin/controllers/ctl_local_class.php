<?php
/**
 * 地方服务分类管理控制器
 *
 * @author eric <0o0zzyZ@gmail.com>
 * @version    $Id: ctl_local_class.php 1541 2009-12-11 07:54:41Z syh $
 */
!defined('PATH_ADMIN') && exit('Forbidden');

class ctl_local_class
{
    /**
     * 分类列表
     */
    public function index()
    {
        try
        {
            app_tpl::assign( 'npa', array('专题管理', '地方服务分类列表') );
            if( isset($_POST['commit']) && $_POST['commit'] == 1)
            {
                if( $_GET['type'] == 'home' )
                {
                    $_POST['io'] = true;
                }
                mod_local_class::update_class( $_POST,  $_POST['action'] );
                mod_login::message("保存成功!");
            }
            $class_id =  isset($_GET['classid']) ? $_GET['classid'] : '';

            $class_list = mod_local_class::get_subclass_list(0);

            app_tpl::assign( 'classid', $class_id );
            app_tpl::assign( 'type', $type );
            app_tpl::assign( 'class_list', $class_list );
            app_tpl::assign( 'option_toggle', array( 0 => '否', 1 => '是' ) );
        }
        catch( Exception $e )
        {
            app_tpl::assign('error', $e->getMessage());
        }

    }

    /**
     * 添加分类
     */
    public function add()
    {
        try
        {
            app_tpl::assign( 'npa', array('专题管理', '新增地方服务分类') );
            if(isset($_POST['classnewname']))
            {
                mod_local_class::add_class( $_POST );
                if(!empty($_POST['mkhtml']))
                {
                    //同时生成页面
                }
            }
            app_tpl::assign( 'class_list', mod_local_class::get_subclass_list(0) );
            app_tpl::assign( 'action', 'add' );
        }
        catch( Exception $e )
        {
            app_tpl::assign('error', $e->getMessage());
        }
    }

    /**
     * 修改分类
     */
    public function edit()
    {
        try
        {
            app_tpl::assign( 'npa', array('专题管理', '编辑地方服务分类') );
            $id = isset($_GET['id']) ? intval($_GET['id']) : '';
            if( $_POST )
            {
                mod_local_class::edit_class( $_POST );
                if(!empty($_POST['mkhtml']))
                {
                    //同时生成页面
                }
                $_GET['classid'] =  isset($_POST['classid']) ? $_POST['classid'] : '';
            }

            app_tpl::assign( 'action', 'edit' );
            app_tpl::assign( 'classid', $_GET['classid']);
            app_tpl::assign( 'type', $_GET['type'] );
            app_tpl::assign( 'returnid', $_GET['classid'] );
            app_tpl::assign( 'info', mod_local_class::get_a_class( $id ) );
            app_tpl::assign( 'class_list', mod_local_class::get_subclass_list(0) );
        }
        catch( Exception $e )
        {
            app_tpl::assign('error', $e->getMessage());
        }
    }

    /**
     * 删除分类
     */
    public function del()
    {
        try
        {
            if( !isset($_GET['id']) )
            {
                throw new Exception('id不能为空');
            }
            mod_local_class::delete_class_and_update_cache( intval($_GET['id']) );
        }
        catch( Exception $e )
        {
            app_tpl::assign('error', $e->getMessage());
        }
    }

    /**
     * 搜索分类
     */
    public function search()
    {
        try
        {
            if( isset($_GET['k']) )
            {
                header("Content-type: text/html; charset=utf-8");
                echo json_encode(mod_local_class::search_class($_GET['k']));
            }
            exit;
        }
        catch( Exception $e )
        {
            app_tpl::assign('error', $e->getMessage());
        }
    }

    /**
     * ajax获取分类列表
     */
    public function ajax_get_list()
    {
        if( isset($_GET['id']) )
        {
            $id = intval($_GET['id']);
            header("Content-type: text/html; charset=utf-8");
            $result = mod_local_class::get_subclass_list($_GET['id']);
            if(empty($result))
            {
                echo json_encode($result);
                exit;
            }
            foreach($result as &$tmp)
            {
                $tmp['classname'] = $tmp['classname'];
            }
            echo json_encode($result);
        }
        exit;
    }

    /**
     * post钩子方法
     */
    public function post()
    {
        try
        {
            app_tpl::display( 'local_class_list.tpl' );
        }
        catch( Exception $e )
        {
            app_tpl::assign('error', $e->getMessage());
        }
    }
}
?>
