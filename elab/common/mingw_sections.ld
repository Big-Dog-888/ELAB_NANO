SECTIONS
{
    elab_export : ALIGN(4)
    {
        PROVIDE(__start_elab_export = .);
        KEEP(*(elab_export))
        PROVIDE(__stop_elab_export = .);
    }

    expoll : ALIGN(4)
    {
        PROVIDE(__start_expoll = .);
        KEEP(*(expoll))
        PROVIDE(__stop_expoll = .);
    }

    shellCommand : ALIGN(4)
    {
        PROVIDE(__start_shellCommand = .);
        KEEP(*(shellCommand))
        PROVIDE(__stop_shellCommand = .);
    }
}