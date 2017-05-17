#define func_p <%=func_name%>_p

static <%=func_name%>_t func_p = 0;

static void
<%=c_iter%>(na_loop_t *const lp)
{
    char *p1, *p2;
    size_t n;
    ssize_t s1, s2;
    dtype  *g;

    INIT_COUNTER(lp,n);
    INIT_PTR(lp,0,p1,s1);
    INIT_PTR(lp,1,p2,s2);
    g = (dtype*)(lp->opt_ptr);

    (*func_p)(n, (dtype*)p1, s1/sizeof(dtype),
                 (dtype*)p2, s2/sizeof(dtype), g);
}

/*<%
 params = [
   param("x",1),
   param("y",1),
 ].select{|x| x}.join("\n  ")
%>
  @overload <%=name%>( x, y, param )
  <%=params%>
  @param [Array,NArray]  array of [DFLAG,DH11,DH21,DH12,DH22]
  @return [<%=class_name%>,<%=class_name%>] returns [x,y]

<%=description%>

 */
static VALUE
<%=c_func(3)%>(VALUE UNUSED(mod), VALUE x, VALUE y, VALUE param)
{
    dtype *g;
    narray_t *na1, *na2, *nap;
    ndfunc_arg_in_t ain[2] = {{OVERWRITE,0},{OVERWRITE,0}};
    ndfunc_t ndf = {<%=c_iter%>, STRIDE_LOOP, 2,0, ain,0};

    CHECK_FUNC(func_p,"<%=func_name%>");

    COPY_OR_CAST_TO(x,cT);
    COPY_OR_CAST_TO(y,cT);
    GetNArray(x,na1);
    GetNArray(y,na2);
    CHECK_DIM_GE(na1,1);
    CHECK_DIM_GE(na2,1);
    CHECK_NON_EMPTY(na1);
    CHECK_NON_EMPTY(na2);
    CHECK_SAME_SHAPE(na1,na2);

    param = rb_funcall(cT,rb_intern("cast"),1,param);
    GetNArray(param,nap);
    CHECK_DIM_EQ(nap,1);
    CHECK_SIZE_GE(nap,5);
    g = (dtype*)nary_get_pointer_for_read(param);

    na_ndloop3(&ndf, g, 2, x, y);

    RB_GC_GUARD(param);
    return rb_assoc_new(x,y);
}

#undef func_p
